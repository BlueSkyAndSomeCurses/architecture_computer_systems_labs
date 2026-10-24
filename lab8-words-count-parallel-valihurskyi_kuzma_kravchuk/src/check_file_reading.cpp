#include "check_file_reading.h"

#include <fstream>
#include <iostream>

#include "options_parser.h"

void check_file_opening(const std::ifstream &file) {
  if (!file.is_open() || !file.good()) {
    std::cerr << "помилка читання файлу конфігурації" << std::endl;
    std::exit(5);
  }
}

std::string check_file_reading(const int argc, char *argv[]) {
  command_line_options_t command_line_options_t{argc, argv};

  std::vector<std::string> cl_arguments = command_line_options_t.get_filenames();

  if (cl_arguments.size() != 1) {
    std::cerr << "неправильна кількість аргументів командного рядка" << std::endl;
    std::exit(1);
  }

  try {
    assert_file_exist(cl_arguments[0]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "не вдалося відкрити файл із конфігурацією" << e.what() << std::endl;
    std::exit(3);
  }

  std::ifstream file(cl_arguments[0]);
  check_file_opening(file);
  file.close();

  return std::string{cl_arguments[0]};
}
