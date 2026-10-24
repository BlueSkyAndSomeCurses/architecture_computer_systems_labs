#include "integrate.h"
#include  <iostream>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/math/special_functions/next.hpp>
#include <cmath>
#include <random>
#include "timer.h"
#include "additional_integral.h"
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
    init_steps_x{init_steps_x}, init_steps_y{init_steps_y}, max_iter{max_iter}, func_to_int{func_num - 1}, gen{std::random_device{}()},
          dist_x{x_start, x_end},
          dist_y{y_start, y_end},
          monte_area{(x_end - x_start) * (y_end - y_start)},
          current_func{f_xy[func_to_int]} {
  x_step = (x_end - x_start) / init_steps_x;
  y_step = (y_end - y_start) / init_steps_y;
};


double Integral::calc_function(int iteration, double prev_area) {
  double res = 0;

  if (iteration == 0) {
    res = x_step * y_step * rect_method(iteration);
    x_step /= 2;
    y_step /= 2;
    init_steps_x = init_steps_x << 1;
    init_steps_y = init_steps_y << 1;

    return res;
  }


  res = prev_area / 4 + rect_method(iteration) * x_step * y_step;
  x_step /= 2;
  y_step /= 2;
  init_steps_x = init_steps_x << 1;
  init_steps_y = init_steps_y << 1;

  return res;
}

double Integral::rect_method(int iteration) {
  double res = 0;


  for (size_t i = 0; i < init_steps_x; ++i) {
    for (size_t j = 0; j < init_steps_y; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      res += f_xy[func_to_int](x_start + i * x_step, y_start + j * y_step);
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

std::pair<double, double> Integral::additional_calc(double x_step, double y_step, int iteration, double prev_area_x, double prev_area_y) {

  double x_res = calc_function_additional(x_step, x_start, x_end, iteration, prev_area_x);
  double y_res = calc_function_additional(y_step, y_start, y_end, iteration, prev_area_y);

  return {x_res, y_res};
}

double Integral::calc_function_additional(double step, double start, double end, int iteration, double prev_area) {

  if (iteration == 0) {
    return step * rect_method_additional(step, start, end);
  }

  double second_half = rect_method_additional(step / pow(2, iteration - 1),  start + step / pow(2, iteration), end);

  return prev_area / 2 + second_half * step / pow(2, iteration);
}


double Integral::rect_method_additional(double step, double new_start, double end) {

  double res = 0;
  double c = 0;

  for (double x = new_start; x < end; x += step) {
    double d = additional_function_part(x) + c;
    auto [new_sum, t] = fast2Sum(res, d);
    res = new_sum;
    c = t;
  }

  return res;
}

std::pair<std::vector<double>, size_t> Integral::additional_function_results() {
  auto start_time = get_current_time_fenced();

  auto i_d_x = (x_end - x_start) / init_steps_x;
  auto j_d_y = (y_end - y_start) / init_steps_y;

  auto [res_x, res_y] = additional_calc(i_d_x, j_d_y, 0, 0, 0);

  auto [new_res_x, new_res_y] = additional_calc(i_d_x, j_d_y, 1, res_x, res_y);

  double new_res = - new_res_x * new_res_y;
  double res = - res_x * res_y;

  auto abs_diff_err = std::abs(new_res - res);

  auto rel_diff_err = boost::math::relative_difference(new_res, res);

  for (int i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res_x = new_res_x;
    res_y = new_res_y;

    std::tie(new_res_x, new_res_y) = additional_calc(i_d_x, j_d_y, i, res_x, res_y);

    new_res = - new_res_x * new_res_y;
    res = - res_x * res_y;

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}

std::pair<std::vector<double>, size_t> Integral::monte_carlo_time() {
  auto start_time = get_current_time_fenced();

  int samples = init_steps_x * init_steps_y;

  double res = monte_carlo_calc(samples, 0, 0);

  int new_samples = samples * 4;

  double new_res = monte_carlo_calc(new_samples, samples, res);

  auto abs_diff_err = std::abs(new_res - res);

  auto rel_diff_err = boost::math::relative_difference(new_res, res);

  for (int i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res = new_res;

    samples = new_samples;
    new_samples *= 4;

    new_res = monte_carlo_calc(new_samples, samples, res);

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}

double Integral::monte_carlo_calc(int samples, int previous_samples, double prev_integ) {
  int new_samples = samples - previous_samples;
  double total_new = monte_carlo(new_samples);
  double total_prev = (prev_integ / monte_area) * previous_samples;
  return (total_prev + total_new) / samples * monte_area;
}

double Integral::monte_carlo(int n_samples) {
  double total = 0.0;
  for (int i = 0; i < n_samples; ++i) {
    double x = dist_x(gen);
    double y = dist_y(gen);
    total += current_func(x, y);
  }
  return total;
}
