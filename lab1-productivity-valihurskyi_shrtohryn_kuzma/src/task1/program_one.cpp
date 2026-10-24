// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "program_one.hpp"
#include "options_parser.h"
#include "timer.hpp"


void standard_idioim(std::string &file_name, std::vector<std::string> &data) {
  std::ifstream file(file_name);
  check_file_opening(file);

  data.emplace_back();
  while (file >> data.back()) {
    data.emplace_back();
  }
}

void read_whole_file(std::string &file_name, std::vector<std::string> &data) {
  std::ifstream file(file_name);
  check_file_opening(file);
  std::stringstream ss;
  ss << file.rdbuf();


  data.emplace_back();
  while (ss >> data.back()) {
    data.emplace_back();
  }
}

void dividing_by_hands(std::string &file_name, std::vector<std::string> &data) {
  std::ifstream file(file_name);

  check_file_opening(file);

  std::stringstream ss;
  ss << file.rdbuf();

  std::string str = ss.str();

  split(data, str, boost::is_any_of("\n"));
}


void solution_really_large_files(std::string &file_name, std::vector<std::string> &data) {
  std::ifstream file(file_name);
  check_file_opening(file);
  auto const starting_pos = file.tellg();

  file.ignore(std::numeric_limits<std::streamsize>::max());
  auto const ignored_chars = file.gcount();

  file.seekg(starting_pos);

  auto str = std::string(ignored_chars, char{});

  file.read(&str.front(), ignored_chars);

  split(data, str, boost::is_any_of("\n"));
}

void really_bad_solution(std::string &file_name, std::vector<std::string> &data) {
  std::ifstream file(file_name);
  check_file_opening(file);
  auto str = std::string{
    std::istreambuf_iterator<char>{file},
    std::istreambuf_iterator<char>{}
  };

  split(data, str, boost::is_any_of("\n\r\t "));
}

size_t letters_count(const std::vector<std::string> &data) {
  size_t count = 0;
  for (const auto &word: data) {
    count += word.size();
  }

  return count;
}
