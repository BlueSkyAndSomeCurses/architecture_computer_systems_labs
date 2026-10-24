#include <iostream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <charconv>
#include <cstdio>


class TransferD {
public:
    static double s_tod(const std::string &str);
    static double s_scanf(const std::string &str);
    static double str_stream(const std::string& str);
    static double a_toi(const std::string& str);
    static double fr_chars(const std::string& str);
};
