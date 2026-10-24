#ifndef PROD_CHECK
#define PROD_CHECK

#include<iostream>
#include<vector>
#include <chrono>
#include <atomic>
#include <functional>
#include <utility>
#include "transfer_funcs.hpp"
#include "timer.hpp"

class TimeChecker {
public:
  static std::pair<std::chrono::nanoseconds, std::size_t> get_time(std::function<unsigned int(std::string)> func,
                                                                   const std::vector<std::string> &vec);

  static std::chrono::nanoseconds get_double_time(std::function<double(std::string)> func,
                                                        const std::vector<std::string> &vec);

  template<class D>
  static inline long long to_us(const D &d) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
  }
};

#endif
