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

namespace fs = std::filesystem;

Archivator::Archivator() {
  archive = archive_read_new();
  archive_read_support_filter_all(archive);
  archive_read_support_format_all(archive);
}

Archivator::~Archivator() {
  archive_read_free(archive);
}


void process_text_with_boost(const std::string &text, oneapi::tbb::concurrent_unordered_map<std::string, int> &word_count) {
  boost::locale::boundary::ssegment_index word_index(
    boost::locale::boundary::word, text.begin(), text.end());
  word_index.rule(boost::locale::boundary::word_letter);

  for (auto it = word_index.begin(); it != word_index.end(); ++it) {
    std::string word = *it;
    word = boost::locale::fold_case(boost::locale::normalize(word));
    word_count[word]++;
  }
}


void Archivator::process_content(const std::string &buff, Config &config, oneapi::tbb::concurrent_unordered_map<std::string, int> &word_count) {
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


void start_pipe_line(Config &config) {
  oneapi::tbb::global_control control(tbb::global_control::max_allowed_parallelism,
                                      config.indexing_threads );

  size_t search_time = 0;
  size_t read_time = 0;

  auto it = fs::recursive_directory_iterator(config.indir);
  auto end_it = fs::recursive_directory_iterator();

  oneapi::tbb::concurrent_unordered_map<std::string, int> global_word_count;

  auto total_start = get_current_time_fenced();

  oneapi::tbb::parallel_pipeline(
    config.filenames_queue_size,
    oneapi::tbb::make_filter<void, fs::path>(
      oneapi::tbb::filter_mode::serial_in_order,
      [&](oneapi::tbb::flow_control &fc) -> fs::path {
        auto start = get_current_time_fenced();
        if (it == end_it) {
          fc.stop();
          return {};
        }

        auto end = get_current_time_fenced();
        search_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return *it++;
      }) &
    oneapi::tbb::make_filter<fs::path, std::pair<std::string, bool> >(
      oneapi::tbb::filter_mode::serial_in_order,
      [&](const fs::path &new_path)-> std::pair<std::string, bool> {
        auto start = get_current_time_fenced();

        char is_file = config.indexing_extensions.contains(new_path.extension()) ? 1 : 0;
        char is_archive = config.archives_extensions.contains(new_path.extension()) ? 1 : 0;

        if (!is_file && !is_archive)
          return {"", 1};

#ifdef DEBUG
        std::cout << new_path << std::endl;
#endif

        std::ifstream raw_file(new_path, std::ios::binary);
        if (!raw_file) {
          throw std::runtime_error("помилка читання архіву " + new_path.string());
        }


        auto raw_text = [&raw_file]() {
          std::ostringstream ss{};
          ss << raw_file.rdbuf();
          return ss.str();
        }();

        auto end = get_current_time_fenced();
        read_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return {raw_text, is_file};
      }) &

    oneapi::tbb::make_filter<std::pair<std::string, bool>, void>
    (oneapi::tbb::filter_mode::parallel, [&]
 (std::pair<std::string, bool> raw_data) {
       Archivator archivator;
       auto is_file = raw_data.second;
       auto binary_string = std::move(
         raw_data.first);

       if (is_file) {
         process_text_with_boost(
           binary_string, global_word_count);
       } else {
         archivator.process_content(
           binary_string, config, global_word_count);
       }

#ifdef DEBUG
       std::cout << "--- file processed! --- " << std::endl;
       for (auto &[word, count]: global_word_count) {
         std::cout << "|" << word << " | " << count << " ";
       }
       std::cout << "\n--- file processed! --- " << std::endl;
#endif
     })
  );

  auto write_start = get_current_time_fenced();
  try {
    write_results_to_file(global_word_count, config);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::exit(6);
  }

  auto total_end = get_current_time_fenced();
  auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();
  auto write_time = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - write_start).count();

  std::cout << "Total=" << total_time << std::endl;
  std::cout << "Finding=" << search_time << std::endl;
  std::cout << "Reading=" << read_time << std::endl;
  std::cout << "Writing=" << write_time << std::endl;
}


void start_hell_machine(Config &config) {
  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);

  start_pipe_line(config);
}
