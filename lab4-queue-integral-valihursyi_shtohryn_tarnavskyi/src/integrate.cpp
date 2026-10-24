#include "integrate.h"
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


  size_t max_iter,
  int func_num, size_t thr_num, size_t prt_num,

  int work_mode
)
  : abs_err{abs_err}, rel_err{rel_err}, x_start{x_start}, x_end{x_end}, y_start{y_start}, y_end{y_end},
    init_steps_x{init_steps_x}, init_steps_y{init_steps_y}, max_iter{max_iter}, func_to_int{func_num - 1},
    threads_number{thr_num}, partition_number{prt_num}, work_mode{work_mode} {
  x_step = (x_end - x_start) / init_steps_x;
  y_step = (y_end - y_start) / init_steps_y;

  workers.resize(threads_number);

  if (work_mode == 0) {
    for (size_t i = 0; i < threads_number; i++) {
      workers[i] = std::thread([&] { cont_calculator(); });
    }
  } else if (work_mode == 1) {
    for (size_t i = 0; i < threads_number; i++) {
      workers[i] = std::thread([&] { cont_calculator_three(); });
    }
  }

  for (size_t i = 0; i < max_iter; ++i) {
    auto *result = new thread_safe_queue<double>();
    outputs_3.push_back(result);
  }
};


double Integral::calc_function(size_t iteration, double prev_area) {
  auto steps_for_each_thread_x = init_steps_x / threads_number;
  auto steps_for_each_thread_y = init_steps_y / partition_number;


  for (size_t i = 0; i < threads_number; ++i) {
    auto x_end_point = (i + 1) * steps_for_each_thread_x;
    if (i == threads_number - 1)
      x_end_point += init_steps_x % threads_number;
    for (size_t j = 0; j < partition_number; ++j) {
      auto y_end_point = (j + 1) * steps_for_each_thread_y;
      if (j == partition_number - 1)
        y_end_point += init_steps_y % partition_number;
      inputs.enqueue({
        iteration, i * steps_for_each_thread_x, x_end_point, j * steps_for_each_thread_y,
        y_end_point
      });
    }
  }

  double res = 0;
  size_t i = 0;
  while (i < threads_number * partition_number) {
    double new_inc;
    outputs.dequeue(new_inc);

    res += new_inc;
    ++i;
  }

  res *= x_step * y_step;

  x_step /= 2;
  y_step /= 2;
  init_steps_x = init_steps_x << 1;
  init_steps_y = init_steps_y << 1;

  return res + prev_area / 4;
}

void Integral::cont_calculator() {
  while (true) {
    std::vector<size_t> new_int;
    inputs.dequeue(new_int);

    auto iter = new_int[0];
    auto x_start_block = new_int[1];
    auto x_end_block = new_int[2];
    auto y_start_block = new_int[3];
    auto y_end_block = new_int[4];


    if (x_start_block > x_end_block) return;

    outputs.enqueue(rect_method(iter, x_start_block, x_end_block, y_start_block, y_end_block));
  }
}

double Integral::rect_method(size_t iteration, size_t x_start_block, size_t x_end_block, size_t y_start_block,
                             size_t y_end_block) {
  double res = 0;

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = y_start_block; j < y_end_block; ++j) {
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

  for (size_t i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res = new_res;

    new_res = calc_function(i, res);

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  for (size_t i = 0; i < threads_number; ++i)
    inputs.enqueue({0, 1, 0, 1, 0});
  for (size_t i = 0; i < threads_number; ++i) workers[i].join();

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}


/// Additional three ///

double Integral::get_res(size_t iteration, double prev_area) {
  double res = 0;
  size_t i = 0;
  double point_number = threads_number * partition_number;

  while (i < point_number) {
    double new_inc;
    outputs_3[iteration]->dequeue(new_inc);

    res += new_inc;
    ++i;
  }

  double mult;

  multipliers.dequeue(mult);

  res *= mult;

  return res + prev_area / 4;
}

std::pair<std::vector<double>, size_t> Integral::res_abserr_relerr_time_three() {
  auto start_time = get_current_time_fenced();

  std::thread th1([this]() { calc_function_three(); });

  double res = get_res(0, 0);

  double new_res = get_res(1, res);

  auto abs_diff_err = std::abs(new_res - res);

  auto rel_diff_err = boost::math::relative_difference(new_res, res);

  for (size_t i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res = new_res;

    new_res = get_res(i, res);

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  stop = true;
  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  for (size_t i = 0; i < threads_number; ++i)
    inputs.enqueue({0, 1, 0, 1, 0, 0, 0});
  for (size_t i = 0; i < threads_number; ++i) workers[i].join();
  th1.join();

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}


void Integral::calc_function_three() {
  for (size_t iter = 0; iter < max_iter; iter++) {
    auto steps_for_each_thread_x = init_steps_x / threads_number;
    auto steps_for_each_thread_y = init_steps_y / partition_number;

    steps.push_back({x_step, y_step});

    for (size_t i = 0; i < threads_number; ++i) {
      auto x_end_point = (i + 1) * steps_for_each_thread_x;
      if (i == threads_number - 1)
        x_end_point += init_steps_x % threads_number;
      for (size_t j = 0; j < partition_number; ++j) {
        auto y_end_point = (j + 1) * steps_for_each_thread_y;
        if (j == partition_number - 1)
          y_end_point += init_steps_y % partition_number;
        inputs.enqueue({
          iter, i * steps_for_each_thread_x, x_end_point, j * steps_for_each_thread_y,
          y_end_point
        });

        if (stop) return;
      }
    }

    multipliers.enqueue(x_step * y_step);
    x_step /= 2;
    y_step /= 2;
    init_steps_x = init_steps_x << 1;
    init_steps_y = init_steps_y << 1;
  }
}

void Integral::cont_calculator_three() {
  while (true) {
    std::vector<size_t> new_int;
    inputs.dequeue(new_int);

    auto iter = new_int[0];
    auto x_start_block = new_int[1];
    auto x_end_block = new_int[2];
    auto y_start_block = new_int[3];
    auto y_end_block = new_int[4];

    if (x_start_block > x_end_block) return;

    outputs_3[iter]->enqueue(rect_method_three(iter, x_start_block, x_end_block, y_start_block, y_end_block));
  }
}

double Integral::rect_method_three(size_t iteration, size_t x_start_block, size_t x_end_block, size_t y_start_block,
                                   size_t y_end_block) {
  double res = 0;
  std::pair<double, double> steps_temp = steps[iteration];
  double x_steps_temp = steps_temp.first;
  double y_steps_temp = steps_temp.second;

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = y_start_block; j < y_end_block; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      res += f_xy[func_to_int](x_start + i * x_steps_temp, y_start + j * y_steps_temp);
    }
  }
  return res;
}

Integral::~Integral() {
  for (const auto &queue_ptr: outputs_3) delete queue_ptr;
}

