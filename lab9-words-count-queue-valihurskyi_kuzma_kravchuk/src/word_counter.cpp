#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <boost/locale.hpp>

#include <oneapi/tbb.h>


#include <boost/lockfree/queue.hpp>
#include <boost/atomic/atomic.hpp>

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


void process_text_with_boost(const std::string &text, word_count_t *word_count) {
  boost::locale::boundary::ssegment_index word_index(
    boost::locale::boundary::word, text.begin(), text.end());
  word_index.rule(boost::locale::boundary::word_letter);

  for (auto it = word_index.begin(); it != word_index.end(); ++it) {
    std::string word = *it;
    word = boost::locale::fold_case(boost::locale::normalize(word));
    ++(*word_count)[word];
  }
}


void Archivator::process_content(const std::string &buff, Config &config, word_count_t *word_count) {
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
#ifdef DEBUG
      std::cout << entry_name << " " << is_archive << std::endl;
#endif
      Archivator().process_content(content, config, word_count);
    }
  }
}


void write_results_to_file(const oneapi::tbb::concurrent_unordered_map<std::string, int> &word_count, Config &config) {
  // Save in alphabetical order
  std::ofstream output_file(config.out_by_a);
  if (!output_file) {
    throw std::runtime_error("помилка запису у вихідний файл: " + config.out_by_n);
  }

  std::vector<std::pair<std::string, int> > word_count_vector(word_count.begin(), word_count.end());
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

void merge_dictionaries(boost::lockfree::queue<word_count_t *> &dictionaries_queue,
                        oneapi::tbb::concurrent_unordered_map<std::string, int> &global_word_count) {
  word_count_t *raw_ptr = nullptr;
  while (true) {
    if (!dictionaries_queue.pop(raw_ptr))continue;

    auto new_word_count = std::unique_ptr<word_count_t>(raw_ptr);

    if (new_word_count->empty()) {
#ifdef DEBUG
      std::cout << "completed merging" << std::endl;
#endif

      return;
    }

    for (auto &[word, count]: *new_word_count) {
      global_word_count[word] += count;
#ifdef DEBUG
      std::cout << word << " : " << count << std::endl;
#endif
    }
  }
}

void unarchive_parse_index(boost::lockfree::queue<std::pair<std::string, char> *> &raw_files_queue,
                           boost::lockfree::queue<word_count_t *> &dictionaries_queue, Config &config) {
  std::pair<std::string, char> *raw_ptr = nullptr;
  while (true) {
    if (!raw_files_queue.pop(raw_ptr)) {
      continue;
    }

    auto bin_str_is_file = std::unique_ptr<std::pair<std::string, char> >(raw_ptr);
    auto &binary_string = bin_str_is_file->first;
    auto is_file = bin_str_is_file->second;

    if (is_file == -1) {
      return;
    }

    auto word_count = new word_count_t();

    Archivator archivator;

    if (is_file) {
      process_text_with_boost(binary_string, word_count);
    } else {
      archivator.process_content(binary_string, config, word_count);
    }

    if (!word_count->empty()) {
#ifdef DEBUG
      std::cout << "--- FILE PROCESSED! --- " << std::endl;
      for (auto &[word, count]: *word_count) {
        std::cout << "|" << word << " | " << count << " ";
      }
      std::cout << "\n--- FILE PROCESSED! --- " << std::endl;
#endif


      dictionaries_queue.push(word_count);
    } else {
      delete word_count;
    };
  }
}

void read_files_from_disk(boost::lockfree::queue<std::pair<std::filesystem::path, char> *> &filenames_queue,
                          boost::lockfree::queue<std::pair<std::string, char> *> &raw_files_queue, Config &config,
                          size_t &reading_time) {
  auto reading_start = get_current_time_fenced();

  std::pair<std::filesystem::path, char> *raw_ptr = nullptr;

  while (true) {
    if (!filenames_queue.pop(raw_ptr)) {
      continue;
    }
    const std::unique_ptr<std::pair<std::filesystem::path, char> > filename_is_file(raw_ptr);
    auto path_to_read = filename_is_file->first;
    auto is_file = filename_is_file->second;
    if (path_to_read.empty()) {
      for (auto i = 0; i < config.indexing_threads; ++i) {
        raw_files_queue.push(new std::pair(std::string{}, static_cast<char>(-1)));
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

    auto *new_pair = new std::pair(std::string{}, is_file);

    new_pair->first = [&raw_file]() {
      std::ostringstream ss{};
      ss << raw_file.rdbuf();
      return ss.str();
    }();


    raw_files_queue.push(new_pair);
  }
}


void read_file_paths_from_disk(boost::lockfree::queue<std::pair<std::filesystem::path, char> *> &filenames_queue,
                               const Config &config) {
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

    auto path_is_file = new std::pair(entry.path(), is_file);

    filenames_queue.push(path_is_file);
  }

  filenames_queue.push(new std::pair(std::filesystem::path(), static_cast<char>(-1)));
}


void start_hell_machine(Config &config) {
  boost::lockfree::queue<std::pair<std::filesystem::path, char> *> filenames_queue(config.filenames_queue_size);
  boost::lockfree::queue<std::pair<std::string, char> *> raw_files_queue(config.raw_files_queue_size);
  boost::lockfree::queue<word_count_t *> dictionaries_queue(config.dictionaries_queue_size);

#ifdef DEBUG
  std::cout << "queues created " << dictionaries_queue.is_lock_free() << std::endl;
#endif

  oneapi::tbb::concurrent_unordered_map<std::string, int> global_word_count;

  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);

  size_t reading_duration = 0;
  std::thread reader_thread(read_files_from_disk, std::ref(filenames_queue), std::ref(raw_files_queue),
                            std::ref(config), std::ref(reading_duration));
#ifdef DEBUG
  std::cout << "reader_thread started" << std::endl;
  std::cout << config.indexing_threads << std::endl;
#endif

  auto indexing_threads = std::vector<std::thread>(config.indexing_threads);

  for (auto i = 0; i < config.indexing_threads; ++i) {
    indexing_threads[i] = std::thread(unarchive_parse_index, std::ref(raw_files_queue), std::ref(dictionaries_queue),
                                      std::ref(config));
  }

  auto merging_threads = std::vector<std::thread>(config.merging_threads);

  for (auto i = 0; i < config.merging_threads; ++i) {
    merging_threads[i] = std::thread(merge_dictionaries, std::ref(dictionaries_queue), std::ref(global_word_count));
  }

  auto total_start = get_current_time_fenced();

  auto start_finding = get_current_time_fenced();

  read_file_paths_from_disk(filenames_queue, config);

  auto end_finding = get_current_time_fenced();

  reader_thread.join();

  std::chrono::milliseconds indexing_duration;
  std::chrono::high_resolution_clock::time_point indexing_start;
  if (config.measure_indexing_time) {
    indexing_start = get_current_time_fenced();
  }

  for (auto i = 0; i < config.indexing_threads; ++i) {
    indexing_threads[i].join();
  }

  if (config.measure_indexing_time) {
    auto indexing_end = get_current_time_fenced();
    indexing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(indexing_end - indexing_start);
  }

  for (auto i = 0; i < config.merging_threads; ++i) {
    dictionaries_queue.push(new word_count_t);
  }


  for (auto i = 0; i < config.merging_threads; ++i) {
    merging_threads[i].join();
  }


#ifdef DEBUG
  std::cout << global_word_count.size() << std::endl;
#endif

  auto write_start = get_current_time_fenced(); // Writing starts here
  write_results_to_file(global_word_count, config);
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
