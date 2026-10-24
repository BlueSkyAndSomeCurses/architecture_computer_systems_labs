// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "transfer_funcs.hpp"

bool Transfer::is_numb(const std::string& str) {
    if (str.empty()) return false;

    for (char c : str) {
        if (!std::isdigit(c))
            return false;
    }
    return true;
}

unsigned int Transfer::str_stream(const std::string& str) {

    unsigned int number = 0;
    std::stringstream sstr(str);

    sstr >> number;

    return number;
}

unsigned int Transfer::a_toi(const std::string& str) {

    return static_cast<unsigned int>(atoi(str.c_str()));
}

unsigned int Transfer::s_tod(const std::string& str) {

    double number = std::stod(str);

    return static_cast<unsigned int>(number);
}

unsigned int Transfer::fr_chars(const std::string& str) {
    unsigned int number = 0;
    std::from_chars(str.data(), str.data() + str.size(), number);

    return number;
}



unsigned int Transfer::s_scanf(const std::string& str) {

    unsigned int number = 0;

    sscanf(str.c_str(), "%u", &number);

    return number;
}
