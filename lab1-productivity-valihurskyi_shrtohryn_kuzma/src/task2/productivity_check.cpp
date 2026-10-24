// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "productivity_check.hpp"
#include "timer.hpp"


std::pair<std::chrono::nanoseconds, std::size_t> TimeChecker::get_time(std::function<unsigned int(std::string)> func,
                                                                       const std::vector<std::string> &vec) {
    size_t sum = 0;
    std::vector<unsigned int> vec_res;
    std::size_t len = vec.size() - 1;

    auto start = get_current_time_fenced();

    for (std::size_t i = 0; i < len; i++)
        vec_res.push_back(func(vec[i]));

    auto end = get_current_time_fenced();

    for (const auto &numb: vec_res)
        sum += numb;

    sum %= 64;

    std::pair<std::chrono::nanoseconds, std::size_t> res = {end - start, sum};

    return res;
}

std::chrono::nanoseconds TimeChecker::get_double_time(std::function<double(std::string)> func,
                                                                       const std::vector<std::string> &vec) {
    std::vector<unsigned int> vec_res;

    auto start = get_current_time_fenced();

    for (const auto &str: vec)
        vec_res.push_back(func(str));

    auto end = get_current_time_fenced();

    return end - start;
}
