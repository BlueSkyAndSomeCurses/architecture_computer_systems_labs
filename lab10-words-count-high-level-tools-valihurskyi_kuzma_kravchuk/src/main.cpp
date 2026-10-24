// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <check_file_reading.h>
#include "options_parser.h"
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <fstream>

#include <oneapi/tbb.h>

#include "config_parser.h"
#include "word_counter.h"


int main(int argc, char *argv[]) {
  auto file_name = check_file_reading(argc, argv);

  auto config = Config(file_name);

  std::filesystem::path root;
  try {
    root = config.indir;
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
      throw std::runtime_error("No such directory " + config.indir);
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::exit(26);
  }

  std::ofstream file(config.out_by_a);
  if (!file.is_open() || !file.good()) {
    std::cerr << "не вдалося відкрити файл для запису результату" << std::endl;
    std::exit(4);
  }
  file.close();

  file.open(config.out_by_n);
  if (!file.is_open() || !file.good()) {
    std::cerr << "не вдалося відкрити файл для запису результату" << std::endl;
    std::exit(4);
  }
  file.close();

  start_hell_machine(config);

  return 0;
}
