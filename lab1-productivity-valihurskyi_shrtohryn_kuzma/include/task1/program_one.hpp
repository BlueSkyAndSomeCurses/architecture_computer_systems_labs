#ifndef PROGRAM1_H
#define PROGRAM1_H
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>

std::pair<std::string, int> read_file(int argc, char *argv[]);

void check_file_opening(const std::ifstream &file);

void standard_idioim(std::string &file_name, std::vector<std::string> &data);

void read_whole_file(std::string &file_name, std::vector<std::string> &data);

void dividing_by_hands(std::string &file_name, std::vector<std::string> &data);

void solution_really_large_files(std::string &file_name, std::vector<std::string> &data);

void really_bad_solution(std::string &file_name, std::vector<std::string> &data);

size_t letters_count(const std::vector<std::string> &data);

#endif
