#ifndef TRANSFER
#define TRANSFER

#include <iostream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <charconv>
#include <cstdio>

class Transfer{
public:
    static unsigned int str_stream(const std::string& str);
    static unsigned int a_toi(const std::string& str);
    static unsigned int s_tod(const std::string& str);
    static unsigned int fr_chars(const std::string& str);
    static unsigned int s_scanf(const std::string& str);
    static bool is_numb(const std::string& str);
};

#endif
