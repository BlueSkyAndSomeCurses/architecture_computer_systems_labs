use crate::config_parser::Config;
use crossbeam_channel::{bounded, Receiver, Sender};

use icu::locid::locale;
use icu::locid::Locale;
use unicode_normalization::UnicodeNormalization;
use unicode_segmentation;
use unicode_segmentation::UnicodeSegmentation;

use std::collections::HashMap;
use std::fs;
use std::io::{Cursor, Read};
use std::path::PathBuf;
use std::thread;
use std::time::SystemTime;
use std::time::UNIX_EPOCH;

use std::cmp::Ordering;
use std::fs::File;
use std::io::Write;

use walkdir::{DirEntry, WalkDir};
use zip::ZipArchive;

#[derive(Debug)]
enum FileType {
    RegularFile,
    Archive,
    PoissonPill,
}

type PathMessage = (PathBuf, FileType);
type RawFileMessage = (Vec<u8>, FileType);

fn is_macos_artifact(entry: &DirEntry) -> bool {
    entry
        .file_name()
        .to_str()
        .map(|s| s.starts_with("._") || s.starts_with("__MACOSX") || s.starts_with(".DS_Store"))
        .unwrap_or(false)
}

type WordCount = HashMap<String, u64>;

fn process_text_with_icu(data: Vec<u8>, word_count: &mut WordCount) {
    let data = String::from_utf8_lossy(&data);

    let iterator = data.unicode_words();
    for word_token in iterator.filter(|word| word.chars().all(|c| !c.is_numeric())) {
        let word_token = word_token.nfkc().collect::<String>().trim().to_lowercase();
        if word_token.is_empty() {
            continue;
        }

        *word_count.entry(word_token).or_insert(0) += 1;
    }
}

fn extract_and_process(data: Vec<u8>, word_count: &mut WordCount, config: &Config) {
    let cursor = Cursor::new(data);
    if let Ok(mut archive) = ZipArchive::new(cursor) {
        for i in 0..archive.len() {
            if let Ok(mut file) = archive.by_index(i) {
                let name = file.name();
                if name.contains("__MACOSX")
                    || name.starts_with("._")
                    || name.contains(".DS_Store")
                    || file.size() > config.max_file_size
                {
                    continue;
                }

                let mut contents = Vec::new();
                if file.read_to_end(&mut contents).is_err() {
                    continue;
                }

                let is_nested_archive = config
                    .archives_extensions
                    .iter()
                    .any(|ext| file.name().ends_with(ext));

                let is_file = config
                    .indexing_extensions
                    .iter()
                    .any(|ext| file.name().ends_with(ext));

                if !is_file && !is_nested_archive {
                    continue;
                }

                if is_nested_archive {
                    extract_and_process(contents, word_count, config);
                } else {
                    process_text_with_icu(contents, word_count);
                };
            }
        }
    }
}

fn read_files_from_disk(
    filenames_recv: Receiver<PathMessage>,
    raw_files_send: Sender<RawFileMessage>,
) {
    loop {
        let (path, file_type) = filenames_recv
            .recv()
            .expect("Error recieving path to file/archive");

        if path.as_os_str().is_empty() {
            break;
        }

        match fs::read(&path) {
            Ok(buff) => raw_files_send.send((buff, file_type)).unwrap(),
            Err(e) => eprintln!("Error reading file {}. {}", path.display(), e),
        }
    }
}

fn unarchive_index_count(
    raw_files_recv: Receiver<RawFileMessage>,
    dict_send: Sender<WordCount>,
    config: &Config,
) {
    while let Ok((data, file_type)) = raw_files_recv.recv() {
        let mut word_count = WordCount::new();
        match file_type {
            FileType::Archive => {
                extract_and_process(data, &mut word_count, config);
            }
            FileType::RegularFile => {
                process_text_with_icu(data, &mut word_count);
            }
            FileType::PoissonPill => {
                return;
            }
        }
        if word_count.is_empty() {
            continue;
        }
        dict_send.send(word_count).unwrap();
    }
}

fn merge_dicts(dict_send: Sender<WordCount>, dict_recv: Receiver<WordCount>) {
    let mut my_word_counter = WordCount::new();
    while let Ok(new_word_count) = dict_recv.recv() {
        if new_word_count.is_empty() {
            dict_send.send(my_word_counter).unwrap();
            return;
        }

        for (word, count) in new_word_count {
            *my_word_counter.entry(word).or_insert(0) += count;
        }
    }
}

fn write_results_to_file(
    word_count: &HashMap<String, u64>,
    config: &Config,
) -> Result<(), Box<dyn std::error::Error>> {
    let mut word_count_vec: Vec<(&String, &u64)> = word_count.iter().collect();
    word_count_vec.sort_by(|a, b| a.0.cmp(b.0));

    let mut output_file = File::create(&config.out_by_a)
        .map_err(|_| format!("помилка запису у вихідний файл: {}", config.out_by_n))?;

    for (word, count) in &word_count_vec {
        writeln!(output_file, "{}: {}", word, count)?;
    }

    word_count_vec.sort_by(|a, b| match b.1.cmp(a.1) {
        Ordering::Equal => a.0.cmp(b.0),
        other => other,
    });

    let mut output_file2 = File::create(&config.out_by_n)
        .map_err(|_| format!("помилка запису у вихідний файл: {}", config.out_by_a))?;

    for (word, count) in &word_count_vec {
        writeln!(output_file2, "{}: {}", word, count)?;
    }

    Ok(())
}

pub fn start_hell_machine(config: &Config) {
    let (filenames_send, filenames_recv) = bounded::<PathMessage>(config.filenames_queue_size);
    let (raw_files_send, raw_files_recv) = bounded::<RawFileMessage>(config.raw_files_queue_size);
    let (dict_send, dict_recv) = bounded::<WordCount>(config.dictionaries_queue_size);

    let locale: Locale = locale!("en-US");

    let tx = raw_files_send.clone();
    let reader_thread = thread::spawn(move || {
        read_files_from_disk(filenames_recv, tx);
    });

    let indexing_threads: Vec<_> = (0..config.indexing_threads)
        .map(|_| {
            let rx = raw_files_recv.clone();
            let dtx = dict_send.clone();
            let cfg_arc = config.clone();
            thread::spawn(move || unarchive_index_count(rx, dtx, &cfg_arc))
        })
        .collect();

    let merging_threads: Vec<_> = (0..config.merging_threads)
        .map(|_| {
            let dtx = dict_send.clone();
            let drx = dict_recv.clone();
            thread::spawn(move || merge_dicts(dtx, drx))
        })
        .collect();

    let total_start = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();

    for entry in WalkDir::new(&config.indir)
        .into_iter()
        .filter_entry(|e| !is_macos_artifact(e))
        .filter_map(Result::ok)
    {
        if !entry.file_type().is_file() {
            continue;
        }

        let path_extension = &entry.path().extension();
        if *path_extension == None {
            continue;
        }

        let is_file = &config.indexing_extensions.contains(
            &path_extension
                .unwrap()
                .to_os_string()
                .into_string()
                .unwrap(),
        );

        let is_archive = &config.archives_extensions.contains(
            &path_extension
                .unwrap()
                .to_os_string()
                .into_string()
                .unwrap(),
        );

        if !is_file && !is_archive {
            continue;
        }

        let file_type = if *is_file {
            FileType::RegularFile
        } else {
            FileType::Archive
        };

        filenames_send
            .send((entry.path().to_path_buf(), file_type))
            .unwrap();
    }
    let find_end = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();

    filenames_send
        .send((PathBuf::new(), FileType::RegularFile))
        .unwrap();

    reader_thread.join().unwrap();

    let read_end = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();
    for _ in 0..config.indexing_threads {
        raw_files_send
            .send((vec![0], FileType::PoissonPill))
            .unwrap();
    }

    for indexing_thread in indexing_threads {
        indexing_thread.join().unwrap();
    }

    for _ in 0..config.merging_threads {
        dict_send.send(WordCount::new()).unwrap();
    }

    for merging_thread in merging_threads {
        merging_thread.join().unwrap();
    }

    let mut resulting_dict = dict_recv.recv().unwrap();

    while !dict_recv.is_empty() {
        let another_dict = dict_recv.recv().unwrap();
        for (word, count) in another_dict {
            *resulting_dict.entry(word).or_insert(0) += count;
        }
    }

    let write_start = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();

    write_results_to_file(&resulting_dict, config).unwrap();

    let total_end = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();

    let total_time = (total_end - total_start).as_millis();
    let find_time = (find_end - total_start).as_millis();
    let read_time = (read_end - total_start).as_millis();
    let write_time = (total_end - write_start).as_millis();

    println!("Total={}", total_time);
    println!("Finding={}", find_time);
    println!("Reading={}", read_time);
    println!("Writing={}", write_time);
}
