#pragma once
#ifndef CHECK_FILE_READING_H
#define CHECK_FILE_READING_H

#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>

#include "matrix.hpp"
#include "options_parser.h"
#include "toml.hpp"
#include <cstddef>

Matrix get_start_points(const std::string &path);

std::unordered_map<int, std::unordered_map<std::pair<int, int>, double, boost::hash<std::pair<int, int>>>> get_hot_points(std::string path, double total_time, double t_step);

std::pair<std::pair<std::vector<int>, std::vector<double>>, std::string> check_file_reading(const std::string& file_name);

#endif
