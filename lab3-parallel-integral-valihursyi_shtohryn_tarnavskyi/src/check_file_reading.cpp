#include <fstream>
#include <iostream>

#include "options_parser.h"
#include "check_file_reading.h"

#include <thread>

bool is_number(const std::string &s) {
  return !s.empty() && std::find_if(s.begin(),
                                    s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void check_file_opening(const std::ifstream &file) {
  if (!file.is_open() || !file.good()) {
    std::cerr << "помилка читання файлу конфігурації" << std::endl;
    std::exit(5);
  }
}

std::pair<std::pair<int, std::string>, int> check_file_reading(const int argc, char *argv[]) {
  command_line_options_t command_line_options_t{argc, argv};

  std::vector<std::string> cl_arguments = command_line_options_t.get_filenames();

  if (cl_arguments.size() != 3) {
    std::cerr << "неправильна кількість аргументів командного рядка" << std::endl;
    std::exit(1);
  }


  if (!is_number(cl_arguments[0])) {
    std::cerr << "неправильний номер функції" << std::endl;
    std::exit(2);
  }

  if (!is_number(cl_arguments[2])) {
    std::cerr << "неправильна кількість потоків" << std::endl;
    std::exit(2);
  }

  int method_number = std::stoi(cl_arguments[0]);
  if (method_number < 1 || method_number > 5) {
    std::cerr << "неправильний номер функції" << std::endl;
    std::exit(2);
  }

  int thread_number = std::stoi(cl_arguments[2]);
  // if (thread_number > std::thread::hardware_concurrency()) {
  //   std::cerr << "кількість потоків перевищує можливості цього процессора" << std::endl;
  //   std::exit(2);
  // }

  try {
    assert_file_exist(cl_arguments[1]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "не вдалося відкрити файл із конфігурацією" << e.what() << std::endl;
    std::exit(3);
  }

  return {{method_number, cl_arguments[1]}, thread_number};
}
