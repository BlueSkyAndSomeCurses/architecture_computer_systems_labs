#include "integrate_precise.h"
#include  <iostream>
#include <boost/math/special_functions/next.hpp>
#include <boost/math/special_functions/relative_difference.hpp>
#include <cmath>
#include "timer.h"
#include "precise_addition.h"

Integral::Integral(
  double abs_err,

  double rel_err,

  double x_start,
  double x_end,
  double y_start,
  double y_end,


  size_t init_steps_x,
  size_t init_steps_y,


  int max_iter,
  int func_num
)
  : abs_err{abs_err}, rel_err{rel_err}, x_start{x_start}, x_end{x_end}, y_start{y_start}, y_end{y_end},
    init_steps_x{init_steps_x}, init_steps_y{init_steps_y}, max_iter{max_iter}, func_to_int{func_num - 1} {
  x_step = (x_end - x_start) / init_steps_x;
  y_step = (y_end - y_start) / init_steps_y;
};


double Integral::calc_function(int iteration, double prev_area) {
  double res = 0;

  if (iteration == 0) {
    res = x_step * y_step * rect_method_precise(iteration);
    x_step /= 2;
    y_step /= 2;
    init_steps_x = init_steps_x << 1;
    init_steps_y = init_steps_y << 1;

    return res;
  }


  res = prev_area / 4 + rect_method_precise(iteration) * x_step * y_step;
  x_step /= 2;
  y_step /= 2;
  init_steps_x = init_steps_x << 1;
  init_steps_y = init_steps_y << 1;

  return res;
}

double Integral::rect_method_precise(int iteration) {
  double res = 0;
  double c = 0;


  for (size_t i = 0; i < init_steps_x; ++i) {
    for (size_t j = 0; j < init_steps_y; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      double b = f_xy[func_to_int](x_start + i * x_step, y_start + j * y_step);
      double d = b + c;
      auto [new_sum, t] = fast2Sum(res, d);
      res = new_sum;
      c = t;
    }
  }

  return res;
}


std::pair<std::vector<double>, size_t> Integral::res_abserr_relerr_time() {
  auto start_time = get_current_time_fenced();

  double res = calc_function(0, 0);

  double new_res = calc_function(1, res);

  auto abs_diff_err = std::abs(new_res - res);

  auto rel_diff_err = boost::math::relative_difference(new_res, res);

  for (int i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res = new_res;

    new_res = calc_function(i, res);

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}

