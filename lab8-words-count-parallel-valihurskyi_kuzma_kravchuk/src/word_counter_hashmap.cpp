#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <boost/locale.hpp>

#include "concurrent_hashmap.h"
#include "word_counter.h"

#include <barrier>
#include <latch>
#include <regex>
#include <thread>

#include "config_parser.h"
#include "thread_safe_queue.h"
#include "timer.h"

Archivator::Archivator() {
  archive = archive_read_new();
  archive_read_support_filter_all(archive);
  archive_read_support_format_all(archive);
}

Archivator::~Archivator() {
  archive_read_free(archive);
}


void process_text_with_boost(const std::string &text, word_count_t &word_count) {
  boost::locale::boundary::ssegment_index word_index(
    boost::locale::boundary::word, text.begin(), text.end());
  word_index.rule(boost::locale::boundary::word_letter);

  for (auto it = word_index.begin(); it != word_index.end(); ++it) {
    std::string word = *it;
    word = boost::locale::fold_case(boost::locale::normalize(word));
    ++word_count[word];
  }
}


void Archivator::process_content(const std::string &buff, Config &config, word_count_t &word_count) {
  if (archive_read_open_memory(archive, buff.c_str(), buff.size()) != ARCHIVE_OK) {
    return;
  }

  archive_entry *entry;
  while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
    const char *entry_name = archive_entry_pathname(entry);
    const std::filesystem::path entry_path = entry_name;
    const auto file_type = archive_entry_filetype(entry);

    auto is_archive = config.archives_extensions.contains(entry_path.extension());
    auto is_textfile = config.indexing_extensions.contains(entry_path.extension());

    if (!is_archive && !is_textfile) {
      archive_read_data_skip(archive);
      continue;
    }
    if (is_textfile && static_cast<size_t>(archive_entry_size(entry)) >= config.max_file_size) {
      archive_read_data_skip(archive);
      continue;
    }


    std::string content;
    if (file_type == AE_IFREG) {
      char buffer[4096];
      la_ssize_t bytes_read;

      while ((bytes_read = archive_read_data(archive, buffer, sizeof(buffer))) > 0) {
        content.append(buffer, bytes_read);
      }
    }

    if (is_textfile) {
      process_text_with_boost(content, word_count);
    }

    if (is_archive) {
      Archivator().process_content(content, config, word_count);
    }
  }
}


void write_results_to_file(const ConcurrentHashMap &word_count, Config &config) {
  std::ofstream output_file(config.out_by_a);
  if (!output_file) {
    throw std::runtime_error("помилка запису у вихідний файл: " + config.out_by_n);
  }

  std::vector<std::pair<std::string, int> > word_count_vector;
  word_count.get_all_elements(word_count_vector);
  std::sort(word_count_vector.begin(), word_count_vector.end(), [](const auto &a, const auto &b) {
    return a.first < b.first;
  });
  for (const auto &pair: word_count_vector) {
    output_file << pair.first << ": " << pair.second << std::endl;
  }
  output_file.close();

  std::sort(word_count_vector.begin(), word_count_vector.end(), [](const auto &a, const auto &b) {
    if (a.second != b.second)
      return a.second > b.second;
    return a.first < b.first;
  });

  std::ofstream output_file2(config.out_by_n);
  if (!output_file2) {
    throw std::runtime_error("помилка запису у вихідний файл: " + config.out_by_a);
  }

  for (const auto &pair: word_count_vector) {
    output_file2 << pair.first << ": " << pair.second << std::endl;
  }
  output_file2.close();
}

void merge_dictionaries(thread_safe_queue<word_count_t> &dictionaries_queue, ConcurrentHashMap &global_dictionary) {
  while (true) {
    auto new_word_count = dictionaries_queue.move_pop();
    if (new_word_count.empty()) {
      return;
    }

    for (auto &[word, count]: new_word_count) {
      global_dictionary[word] += count;
    }
  }
}

void unarchive_parse_index(thread_safe_queue<std::pair<std::string, char> > &raw_files_queue,
                           thread_safe_queue<word_count_t> &dictionaries_queue, Config &config,
                           std::latch &indexing_latch) {
  while (true) {
    auto [binary_string, is_file] = raw_files_queue.move_pop();

    if (is_file == -1) {
      indexing_latch.count_down();
      return;
    }

    word_count_t word_count;

    Archivator archivator;

    if (is_file) {
      process_text_with_boost(binary_string, word_count);
    } else {
      archivator.process_content(binary_string, config, word_count);
    }

    if (!word_count.empty()) {
#ifdef DEBUG
      std::cout << "--- FILE PROCESSED! --- " << std::endl;
      for (auto &[word, count]: word_count) {
        std::cout << "|" << word << " | " << count << " ";
      }
      std::cout << "\n--- FILE PROCESSED! --- " << std::endl;
#endif

      dictionaries_queue.move_push(std::move(word_count));
    };
  }
}

void read_files_from_disk(thread_safe_queue<std::pair<std::filesystem::path, char> > &filenames_queue,
                          thread_safe_queue<std::pair<std::string, char> > &raw_files_queue, Config &config,
                          size_t &reading_time) {
  auto reading_start = get_current_time_fenced();
  while (true) {
    std::pair<std::filesystem::path, char> filename_is_file = filenames_queue.move_pop();
    auto path_to_read = filename_is_file.first;
    auto is_file = filename_is_file.second;
    if (path_to_read.empty()) {
      for (auto i = 0; i < config.indexing_threads; ++i) {
        raw_files_queue.push({"", -1});
      }

      auto reading_end = get_current_time_fenced();

      reading_time = std::chrono::duration_cast<std::chrono::milliseconds>(reading_end - reading_start).count();
      return;
    }

#ifdef DEBUG
    std::cout << path_to_read << std::endl;
#endif

    std::ifstream raw_file(path_to_read, std::ios::binary);
    if (!raw_file) {
      throw std::runtime_error("помилка читання архіву " + path_to_read.string());
    }

    auto buffer = [&raw_file]() {
      std::ostringstream ss{};
      ss << raw_file.rdbuf();
      return ss.str();
    }();

    raw_files_queue.move_push({std::move(buffer), is_file});
  }
}


void read_file_paths_from_disk(thread_safe_queue<std::pair<std::filesystem::path, char>> &filenames_queue,
                               Config &config) {
  for (auto &entry: std::filesystem::recursive_directory_iterator(config.indir)) {
    if (!std::filesystem::is_regular_file(entry)) {
      continue;
    }

    const auto ext = entry.path().extension();
    char is_file = config.indexing_extensions.contains(ext) ? 1 : 0;
    char is_archive = config.archives_extensions.contains(ext) ? 1 : 0;

    if (!is_file && !is_archive) {
      continue;
    }

    filenames_queue.push({entry.path(), is_file});
  }

  // Indicate completion to reader thread
  filenames_queue.push({std::filesystem::path(), -1});
}


void start_hell_machine(Config &config) {
  thread_safe_queue<std::pair<std::filesystem::path, char> > filenames_queue(config.filenames_queue_size);
  thread_safe_queue<std::pair<std::string, char> > raw_files_queue(config.raw_files_queue_size);
  thread_safe_queue<word_count_t> dictionaries_queue(config.dictionaries_queue_size);
  ConcurrentHashMap global_dictionary;

  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);

  size_t reading_duration = 0;
  std::thread reader_thread(read_files_from_disk, std::ref(filenames_queue), std::ref(raw_files_queue),
                            std::ref(config), std::ref(reading_duration));

  auto indexing_threads = std::vector<std::thread>(config.indexing_threads);
  std::latch indexing_latch(config.indexing_threads);

  for (auto i = 0; i < config.indexing_threads; ++i) {
    indexing_threads[i] = std::thread(unarchive_parse_index, std::ref(raw_files_queue), std::ref(dictionaries_queue),
                                      std::ref(config), std::ref(indexing_latch));
  }

  auto merging_threads = std::vector<std::thread>(config.merging_threads);

  for (auto i = 0; i < config.merging_threads; ++i) {
    merging_threads[i] = std::thread(merge_dictionaries, std::ref(dictionaries_queue), std::ref(global_dictionary));
  }

  auto total_start = get_current_time_fenced();

  auto start_finding = get_current_time_fenced();
  for (auto &entry: std::filesystem::recursive_directory_iterator(config.indir)) {
    if (!std::filesystem::is_regular_file(entry)) {
      continue;
    }
    char is_file = config.indexing_extensions.contains(entry.path().extension()) ? 1 : 0;
    char is_archive = config.archives_extensions.contains(entry.path().extension()) ? 1 : 0;

    if (!is_file && !is_archive) {
      continue;
    }

    filenames_queue.push({entry.path(), is_file});
  }
  auto end_finding = get_current_time_fenced();
  filenames_queue.push(std::make_pair(std::filesystem::path(), -1));

  reader_thread.join();

  std::chrono::milliseconds indexing_duration{0};
  if (config.measure_indexing_time) {
    auto indexing_start = get_current_time_fenced();
    indexing_latch.wait();
    auto indexing_end = get_current_time_fenced();
    indexing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(indexing_end - indexing_start);
  } else {
    indexing_latch.wait();
  }

  for (auto i = 0; i < config.merging_threads; ++i) {
    dictionaries_queue.push(word_count_t());
  }

  for (auto i = 0; i < config.indexing_threads; ++i) {
    indexing_threads[i].join();
  }

  for (auto i = 0; i < config.merging_threads; ++i) {
    merging_threads[i].join();
  }

#ifdef DEBUG
  std::cout << dictionaries_queue.size() << std::endl;
#endif

  auto write_start = get_current_time_fenced(); // Writing starts here
  write_results_to_file(global_dictionary, config);
  auto end = get_current_time_fenced(); // Everything ends here

  using namespace std::chrono;
  auto total_duration = duration_cast<milliseconds>(end - total_start).count();
  auto writing_duration = duration_cast<milliseconds>(end - write_start).count();
  auto finding_duration = duration_cast<milliseconds>(end_finding - start_finding).count();

  std::cout << "Total=" << total_duration << std::endl;
  std::cout << "Finding=" << finding_duration << std::endl;
  std::cout << "Reading=" << reading_duration << std::endl;
  std::cout << "Writing=" << writing_duration << std::endl;
  if (config.measure_indexing_time) {
    std::cout << "Indexing=" << indexing_duration.count() << std::endl;
  }
}
