#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>

namespace po = boost::program_options;

class Config {
public:
  std::string indir;
  std::string out_by_a;
  std::string out_by_n;
  std::unordered_set<std::string> indexing_extensions;
  std::unordered_set<std::string> archives_extensions;
  size_t max_file_size = 0;

  int indexing_threads = 0;
  int merging_threads = 0;

  int filenames_queue_size = 0;
  int raw_files_queue_size = 0;
  int dictionaries_queue_size = 0;

  // Додано для динамічного пулу
  int min_indexing_threads = 1;
  int max_indexing_threads = 4;
  int min_merging_threads = 1;
  int max_merging_threads = 2;
  int max_total_threads = 6;
  int adjust_interval_ms = 500;

  bool measure_indexing_time = false;

  explicit Config(const std::string &file_name);
};

#endif // CONFIG_PARSER_H
