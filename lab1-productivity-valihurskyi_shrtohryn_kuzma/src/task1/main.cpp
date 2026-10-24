// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>

#include "check_file_reading.h"
#include "options_parser.h"
#include "program_one.hpp"
#include "productivity_check.hpp"


int main(int argc, char *argv[]) {
  auto [file_name, method_number] = check_file_reading(argc, argv);

  std::vector<std::string> data;

  std::vector<std::function<void(std::string &, std::vector<std::string> &)> > reading_methods_vec = {
    standard_idioim,
    read_whole_file,
    dividing_by_hands,
    solution_really_large_files,
    really_bad_solution
  };

  std::chrono::high_resolution_clock::time_point stage_one_time = get_current_time_fenced();
  reading_methods_vec[method_number - 1](file_name, data);
  std::chrono::high_resolution_clock::time_point stage_two_time = get_current_time_fenced();


  double elapsed_time_ms = static_cast<int>(std::chrono::duration<double, std::milli>(stage_two_time - stage_one_time).
    count());

  std::cout << elapsed_time_ms << '\n' << letters_count(data) << std::endl;

  return 0;
}
