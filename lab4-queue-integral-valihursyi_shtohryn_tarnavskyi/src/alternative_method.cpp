#include "alternative_method.h"
#include  <iostream>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/math/special_functions/next.hpp>
#include <cmath>
#include <thread>
#include "timer.h"

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
  int func_num,
  int thr_num,
  int prt_num


)
  : abs_err{abs_err}, rel_err{rel_err}, x_start{x_start}, x_end{x_end}, y_start{y_start}, y_end{y_end},
    init_steps_x{init_steps_x}, init_steps_y{init_steps_y}, max_iter{max_iter/2}, func_to_int{func_num - 1},
    threads_number{thr_num}, partition_number{prt_num}, expected_num{thr_num * prt_num}  {

  x_step = (x_end - x_start) / init_steps_x;
  y_step = (y_end - y_start) / init_steps_y;

  workers.resize(threads_number);

  for (int i = 0; i < threads_number; i++) {
    workers[i] = std::thread([&] { cont_calculator(); });
  }
};


std::vector<double> Integral::calc_function() {
  auto steps_for_each_thread_x = init_steps_x / threads_number;
  auto steps_for_each_thread_y = init_steps_y / partition_number;


  for (int i = 0; i < threads_number; ++i) {
    auto x_end_point = (i + 1) * steps_for_each_thread_x;
    if (i == threads_number - 1)
      x_end_point += init_steps_x % threads_number;
    for (int j = 0; j < partition_number; ++j) {
      auto y_end_point = (j + 1) * steps_for_each_thread_y;
      if (j == partition_number - 1)
        y_end_point += init_steps_y % partition_number;
      inputs.enqueue({
        0, i * steps_for_each_thread_x, x_end_point, j * steps_for_each_thread_y,
        y_end_point
      });
    }
  }

  double res = 0;
  double res_abs_err = 0;
  double res_rel_err = 0;

  while (expected_num > 0) {
    std::vector<double> out;
    outputs.dequeue(out);

    res += out[0];
    res_abs_err += out[1];
    res_rel_err += out[2];
  }

  res_rel_err = res_rel_err / threads_number / partition_number;

  return {res, res_abs_err, res_rel_err};
}

void Integral::cont_calculator() {
  while (true) {
    std::vector<size_t> new_int;
    inputs.dequeue(new_int);

    auto attempt = new_int[0];
    auto x_start_block = new_int[1];
    auto x_end_block = new_int[2];
    auto y_start_block = new_int[3];
    auto y_end_block = new_int[4];
    auto cur_x_step = x_step / (1 << (attempt * (max_iter - 1)));
    auto cur_y_step = y_step;

    auto target_abs_err = abs_err / threads_number / partition_number / (1 << attempt);
    auto target_rel_err = rel_err;

    if (x_start_block > x_end_block) return;

    auto res = rect_method(0, x_start_block, x_end_block, y_start_block, y_end_block, cur_x_step, cur_y_step);
    res = res * cur_x_step * cur_y_step;

    x_start_block = x_start_block << 1;
    x_end_block = x_end_block << 1;
    y_start_block = y_start_block << 1;
    y_end_block = y_end_block << 1;
    cur_x_step /= 2;
    cur_y_step /= 2;

    auto new_res = rect_method(1, x_start_block, x_end_block, y_start_block, y_end_block, cur_x_step, cur_y_step);
    new_res = new_res * cur_x_step * cur_y_step + res / 4;

    auto cur_abs_err = std::abs(new_res - res);
    auto cur_rel_err = boost::math::relative_difference(new_res, res);

    for (int iter = 2; (cur_abs_err > target_abs_err || cur_rel_err > target_rel_err) && iter < max_iter; ++iter) {
      res = new_res;

      x_start_block = x_start_block << 1;
      x_end_block = x_end_block << 1;
      y_start_block = y_start_block << 1;
      y_end_block = y_end_block << 1;
      cur_x_step /= 2;
      cur_y_step /= 2;

      new_res = rect_method(iter, x_start_block, x_end_block, y_start_block, y_end_block, cur_x_step, cur_y_step);
      new_res = new_res * cur_x_step * cur_y_step + res / 4;

      cur_abs_err = std::abs(new_res - res);
      cur_rel_err = boost::math::relative_difference(new_res, res);
    }

    if (cur_abs_err > target_abs_err || cur_rel_err > target_rel_err) {

      ++expected_num;

      inputs.enqueue({attempt + 1, x_start_block, (x_start_block + x_end_block) / 2, new_int[3], new_int[4]});
      inputs.enqueue({attempt + 1, (x_start_block + x_end_block) / 2, x_end_block, new_int[3], new_int[4]});

    } else {

      outputs.enqueue({new_res, cur_abs_err, cur_rel_err / (1 << attempt)});
      --expected_num;
    }

  }
}

double Integral::rect_method(size_t iteration, size_t x_start_block, size_t x_end_block, size_t y_start_block,
                             size_t y_end_block, double cur_x_step, double cur_y_step) {
  double res = 0;

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = y_start_block; j < y_end_block; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      res += f_xy[func_to_int](x_start + i * cur_x_step, y_start + j * cur_y_step);
    }
  }
  return res;
}


std::pair<std::vector<double>, size_t> Integral::res_abserr_relerr_time() {
  auto start_time = get_current_time_fenced();

  std::vector<double> out = calc_function();
  auto new_res = out[0];
  auto abs_diff_err = out[1];
  auto rel_diff_err = out[2];

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  for (int i = 0; i < threads_number; ++i)
    inputs.enqueue({0, 1, 0, 1, 0});
  for (int i = 0; i < threads_number; ++i) workers[i].join();

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}