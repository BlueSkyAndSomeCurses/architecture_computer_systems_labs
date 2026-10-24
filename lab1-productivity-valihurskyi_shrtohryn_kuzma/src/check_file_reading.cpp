// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <fstream>
#include <iostream>

#include "options_parser.h"
#include "check_file_reading.h"

bool is_number(const std::string &s) {
  return !s.empty() && std::find_if(s.begin(),
                                    s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void check_file_opening(const std::ifstream &file) {
  if (!file.is_open() || !file.good()) {
    std::cerr << "помилка читання вхідного файлу" << std::endl;
    std::exit(5);
  }
}

std::pair<std::string, int> check_file_reading(int argc, char *argv[]) {
  command_line_options_t command_line_options_t{argc, argv};

  std::vector<std::string> cl_arguments = command_line_options_t.get_filenames();

  if (cl_arguments.size() != 2) {
    std::cerr << "неправильна кількість аргументів командного рядка" << std::endl;
    std::exit(1);
  }


  if (!is_number(cl_arguments[1])) {
    std::cerr << "неправильний номер методу" << std::endl;
    std::exit(2);
  }

  int method_number = std::stoi(cl_arguments[1]);
  if (method_number < 1 || method_number > 5) {
    std::cerr << "неправильний номер методу" << std::endl;
    std::exit(2);
  }

  try {
    assert_file_exist(cl_arguments[0]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "не вдалося відкрити файл із вхідними даними\n" << e.what() << std::endl;
    std::exit(3);
  }

  return {cl_arguments[0], method_number};
}
