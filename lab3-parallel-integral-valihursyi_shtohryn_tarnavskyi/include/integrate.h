#ifndef INTEGRATE_H
#define INTEGRATE_H

#include <functional>
#include <vector>
#include "functions_to_integrate.h"
#include <atomic>
#include <mutex>

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

  int max_iter;

  int func_to_int;
  int threads_number;

  std::mutex mtx;

  std::vector<std::function<double(double, double)> > f_xy = {de_johnge, ackleys_function, langermann_function};

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

    int max_iter, int func_num, int thr_num
  );

  double calc_function(int iteration, double prev_area,
    std::function<void(int, size_t, size_t, std::atomic<double> &)> rect_func);

  void rect_method(int iteration, size_t x_start_block, size_t x_end_block,
                   std::atomic<double> &gen_res);

  std::pair<std::vector<double>, size_t> res_abserr_relerr_time(int func_number = 1);

  double calc_function_mutex(int iteration, double prev_area,
  std::function<void(int, size_t, size_t, double &)> rect_func);

  std::pair<std::vector<double>, size_t> res_abserr_relerr_time_mutex(
  int func_number = 1);

  void rect_method_mutex(int iteration, size_t x_start_block,
                         size_t x_end_block, double &gen_res);

  void rect_method_mutex_bad(int iteration, size_t x_start_block,
                        size_t x_end_block, double &gen_res);

  void rect_method_atomic_bad(int iteration, size_t x_start_block,
                              size_t x_end_block, std::atomic<double> &gen_res);
};


#endif //INTEGRATE_H
