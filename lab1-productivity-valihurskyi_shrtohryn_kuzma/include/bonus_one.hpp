#ifndef BONUS1_HPP
#define BONUS1_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <qt6/QtCore/QString>
#include <qt6/QtCore/QStringList>
#include <absl/strings/str_split.h>
#include <absl/strings/numbers.h>
#include "options_parser.h"
#include "timer.hpp"

// Test splitting a buffer into a vector of strings

void boost_parse(const std::string &str, std::vector<std::string> &data);

void qt_split(const QString &str, QStringList &data);

void absl_split(const std::string &str, std::vector<std::string> &data);

// Test converting split strings into numbers

void boost_convert(const std::vector<std::string> &data, std::vector<unsigned int> &numbers);

void qt_to_uint(const QStringList &data, std::vector<unsigned int> &numbers);

void absl_atoi(const std::vector<std::string> &data, std::vector<unsigned int> &numbers);

#endif // BONUS1_HPP
