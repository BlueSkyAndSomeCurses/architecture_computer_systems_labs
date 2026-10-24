#include <fstream>
#include <iostream>

#include "productivity_check.hpp"
#include "transfer_funcs_d.hpp"
#include "options_parser.h"
#include "program_one.hpp"
#include "check_file_reading.h"


int main(int argc, char *argv[]) {
    auto [file, method_number] = check_file_reading(argc, argv);

    std::vector<std::string> vec;

    solution_really_large_files(file, vec);

    std::vector<std::function<int(std::string)>> func_vec = {
        TransferD::str_stream,
        TransferD::a_toi,
        TransferD::s_tod,
        TransferD::fr_chars,
        TransferD::s_scanf
    };

    auto res = TimeChecker::get_double_time(func_vec[method_number - 1], vec);

    std::cout << TimeChecker::to_us(res) << std::endl;

    return 0;
}
