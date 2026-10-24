#ifndef INTEGRATE_H
#define INTEGRATE_H

#include <functional>
#include <vector>
#include <thread>
#include <thread_safe_queue.h>
#include <mutex>
#include <iostream>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/math/special_functions/next.hpp>
#include <cmath>
#include <future>
#include <thread>
#include <unordered_map>
#include "timer.h"
#include "functions_to_integrate.h"

class Integral {
  double abs_err;
  double rel_err;

  double x_start;
  double x_end;
  double y_start;
  double y_end;

  double x_step;
  double y_step;

  size_t init_steps_x;
  size_t init_steps_y;

  size_t max_iter;

  int func_to_int;
  size_t threads_number;
  size_t partition_number;

  thread_safe_queue<std::vector<size_t> > inputs;
  thread_safe_queue<double> outputs;
  std::vector<std::function<double(double, double)> > f_xy = {de_johnge, ackleys_function, langermann_function};
  std::vector<std::thread> workers;

  int work_mode;
  thread_safe_queue<double> results;
  thread_safe_queue<double> multipliers;
  std::vector<thread_safe_queue<double> *> outputs_3;
  std::condition_variable cv3_start;
  std::condition_variable cv3;
  std::mutex mtx3;
  bool cont_counter = true;
  bool starter = false;
  bool stop = false;
  std::vector<std::pair<double, double> > steps;

public:
  Integral(
    double abs_err,

    double rel_err,

    double x_start,
    double x_end,
    double y_start,
    double y_end,

    size_t init_steps_x,
    size_t init_steps_y,

    size_t max_iter, int func_num, size_t thr_num, size_t prt_num,

    int work_mode = 0);

  ~Integral();

  double calc_function(size_t iteration, double prev_area);

  void cont_calculator();

  double rect_method(size_t iteration, size_t x_start_block, size_t x_end_block, size_t y_start_block,
                     size_t y_end_block);

  std::pair<std::vector<double>, size_t> res_abserr_relerr_time();

  std::pair<std::vector<double>, size_t> res_abserr_relerr_time_three();

  void cont_calculator_three();

  void calc_function_three();

  double get_res(size_t iteration, double prev_area);

  double rect_method_three(size_t iteration, size_t x_start_block, size_t x_end_block, size_t y_start_block,
                           size_t y_end_block);
};


#endif //INTEGRATE_H
