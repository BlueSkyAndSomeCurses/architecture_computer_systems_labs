// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <fstream>
#include <iostream>

#include "productivity_check.hpp"
#include "transfer_funcs.hpp"
#include "options_parser.h"
#include "program_one.hpp"
#include "check_file_reading.h"


int main(int argc, char *argv[]) {
    auto [file, method_number] = check_file_reading(argc, argv);

    std::vector<std::string> vec;

    solution_really_large_files(file, vec);

    std::vector<std::function<int(std::string)>> func_vec = {
        Transfer::str_stream,
        Transfer::a_toi,
        Transfer::s_tod,
        Transfer::fr_chars,
        Transfer::s_scanf
    };

    auto res = TimeChecker::get_time(func_vec[method_number - 1], vec);

    std::cout << TimeChecker::to_us(res.first) << std::endl;
    std::cout << res.second << std::endl;

    return 0;
}
