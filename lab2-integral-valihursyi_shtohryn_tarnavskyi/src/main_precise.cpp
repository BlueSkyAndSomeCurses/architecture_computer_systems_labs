#include <iostream>

#include "check_file_reading.h"
#include "integrate_precise.h"
#include "toml.hpp"


int main(int argc, char *argv[]) {
    auto [method_num, file_name] = check_file_reading(argc, argv);

    toml::table tbl;
    try {
        tbl = toml::parse_file(file_name);
    } catch (const toml::parse_error &err) {
        std::cerr << "помилка читання файлу конфігурації, або помилка його вмісту" << err << "\n";
        std::exit(5);
    }


    auto abs_err = tbl["abs_err"].value<double>().value();
    auto rel_err = tbl["rel_err"].value<double>().value();
    auto x_start = tbl["x_start"].value<double>().value();
    auto x_end = tbl["x_end"].value<double>().value();
    auto y_start = tbl["y_start"].value<double>().value();
    auto y_end = tbl["y_end"].value<double>().value();
    auto init_steps_x = tbl["init_steps_x"].value<double>().value();
    auto init_steps_y = tbl["init_steps_y"].value<double>().value();
    auto max_iter = tbl["max_iter"].value<int>().value();

    Integral integral(abs_err, rel_err, x_start, x_end, y_start, y_end, init_steps_x, init_steps_y, max_iter,
                      method_num);

    auto [res_abs_rel, calc_time] = integral.res_abserr_relerr_time();
    std::cout << res_abs_rel[0] << std::endl;
    std::cout << res_abs_rel[1] << std::endl;
    std::cout << res_abs_rel[2] << std::endl;
    std::cout << calc_time << std::endl;

    return 0;
}