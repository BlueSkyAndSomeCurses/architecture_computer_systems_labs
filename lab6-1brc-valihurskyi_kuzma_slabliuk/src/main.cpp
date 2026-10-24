// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <check_file_reading.h>
#include "brc_logic.h"
#include "options_parser.h"
#include "timer.h"

#include "toml.hpp"


int main(int argc, char *argv[]) {
    auto file_name = check_file_reading(argc, argv);

    toml::table tbl;
    try {
        tbl = toml::parse_file(file_name);
    } catch (const toml::parse_error &err) {
        std::cerr << "помилка читання файлу конфігурації, або помилка його вмісту" << err << "\n";
        std::exit(5);
    }

    std::string infile, outfile;
    int threads_num;
    try {
        infile = tbl["infile"].value<std::string>().value();
    } catch (const std::bad_optional_access &err) {
        std::cerr << "відсутній рядок infile" << "\n";
        std::exit(5);
    }

    try {
        outfile = tbl["outfile"].value<std::string>().value();
    } catch (const std::bad_optional_access &err) {
        std::cerr << "відсутній рядок outfile" << "\n";
        std::exit(5);
    }

    try {
        threads_num = tbl["threads"].value<int>().value();
    } catch (const std::bad_optional_access &err) {
        std::cerr << "відсутній рядок threads" << "\n";
        std::exit(5);
    }

    std::unordered_map<std::string, StationObservations> observations;
    try {
        assert_file_exist(infile);
    } catch (const std::invalid_argument &e) {
        std::cerr << "infile does not exist\n";
        std::exit(26);
    }

    std::chrono::high_resolution_clock::time_point stage_one_time = get_current_time_fenced();
    size_t read_time = process_observations(observations, infile, threads_num);

    size_t write_time;
    try {
        write_time = write_results(observations, outfile);
    } catch (const std::exception &err) {
        std::cerr << "помилка запису у вихідний файл" << "\n";
        std::exit(6);
    }

    std::chrono::high_resolution_clock::time_point stage_two_time = get_current_time_fenced();
    size_t elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
            stage_two_time - stage_one_time).
        count());

    std::cout << "Total=" << elapsed_time_ms << '\n';
    std::cout << "Reading=" << read_time << '\n';
    std::cout << "Writing=" << write_time << '\n';

    return 0;
}
