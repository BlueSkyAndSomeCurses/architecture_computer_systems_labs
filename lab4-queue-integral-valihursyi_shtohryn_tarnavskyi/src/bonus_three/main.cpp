// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <check_file_reading.h>

#include "integrate.h"
#include "toml.hpp"


int main(int argc, char *argv[]) {
    auto [method_thread_partions, file_name] = check_file_reading(argc, argv);
    auto method_num = method_thread_partions[0];
    auto threads_number = method_thread_partions[1];
    auto partion_number = method_thread_partions[2];

    toml::table tbl;
    try {
        tbl = toml::parse_file(file_name);
    } catch (const toml::parse_error &err) {
        std::cerr << "помилка читання файлу конфігурації, або помилка його вмісту" << err << "\n";
        std::exit(5);
    }


    double abs_err, rel_err, x_start, x_end, y_start, y_end;
    size_t init_steps_x, init_steps_y;
    int max_iter;
    try {
        abs_err = tbl["abs_err"].value<double>().value();
        rel_err = tbl["rel_err"].value<double>().value();
        x_start = tbl["x_start"].value<double>().value();
        x_end = tbl["x_end"].value<double>().value();
        y_start = tbl["y_start"].value<double>().value();
        y_end = tbl["y_end"].value<double>().value();
        init_steps_x = tbl["init_steps_x"].value<size_t>().value();
        init_steps_y = tbl["init_steps_y"].value<size_t>().value();
        max_iter = tbl["max_iter"].value<int>().value();
    } catch (const std::bad_optional_access &err) {
        std::cerr << "неправильна структура файлу конфігурації" << "\n";
        std::exit(5);
    }

    Integral integral(abs_err, rel_err, x_start, x_end, y_start, y_end, init_steps_x, init_steps_y, max_iter,
                      method_num, threads_number, partion_number, 1);

    auto [res_abs_rel, calc_time] = integral.res_abserr_relerr_time_three();


    if (res_abs_rel[1] > abs_err || res_abs_rel[2] > rel_err) {
        std::cerr << "не вдалося досягнути бажаної точності\n";
        std::exit(16);
    }

    std::cout << res_abs_rel[0] << std::endl;
    std::cout << res_abs_rel[1] << std::endl;
    std::cout << res_abs_rel[2] << std::endl;
    std::cout << calc_time << std::endl;

    return 0;
}
