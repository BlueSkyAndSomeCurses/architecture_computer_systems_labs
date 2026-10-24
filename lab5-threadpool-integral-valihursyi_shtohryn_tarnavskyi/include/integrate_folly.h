#ifndef INTEGRATE_FOLLY_H
#define INTEGRATE_FOLLY_H

#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/math/special_functions/next.hpp>
#include <cmath>
#include <future>
#include <thread>
#include <unordered_map>
#include <array>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>

#include "timer.h"
#include "functions_to_integrate.h"
#include "thread_safe_queue.h"
#include "thread_pool.hpp"
#include "task.hpp"

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

  std::vector<std::function<double(double, double)> > f_xy = {de_johnge, ackleys_function, langermann_function};
  std::unique_ptr<folly::CPUThreadPoolExecutor> thread_pool;

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

      size_t max_iter, int func_num, size_t thr_num, size_t prt_num
    );


  double calc_function(size_t iteration, double prev_area);

  double rect_method(const std::array<size_t, 5> &args);

  std::pair<std::vector<double>, size_t> res_abserr_relerr_time();
};


#endif //INTEGRATE_FOLLY_H
