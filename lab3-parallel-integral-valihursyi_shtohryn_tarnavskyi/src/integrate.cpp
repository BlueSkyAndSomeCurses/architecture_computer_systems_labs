#include "integrate.h"
#include  <iostream>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/math/special_functions/next.hpp>
#include <cmath>
#include <thread>
#include <functional>
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
    int func_num, int thr_num)
    : abs_err{abs_err}, rel_err{rel_err}, x_start{x_start}, x_end{x_end}, y_start{y_start}, y_end{y_end},
      init_steps_x{init_steps_x}, init_steps_y{init_steps_y}, max_iter{max_iter}, func_to_int{func_num - 1},
      threads_number{thr_num}
{
  x_step = (x_end - x_start) / init_steps_x;
  y_step = (y_end - y_start) / init_steps_y;
};

double Integral::calc_function(int iteration, double prev_area,
  std::function<void(int, size_t, size_t, std::atomic<double> &)> rect_func) {

  std::atomic<double> res(0);

  std::vector<std::thread> workers(threads_number - 1);
  auto steps_for_each_thread = init_steps_x / threads_number;
  for (int i = 0; i < threads_number - 1; ++i) {
    workers[i] = std::thread([=, &res] {
        auto x_start_block = i * steps_for_each_thread;
        auto x_end_block = (i + 1) * steps_for_each_thread;
        rect_func(iteration, x_start_block, x_end_block, std::ref(res));
      }
    );
  }

  rect_func(iteration, (threads_number - 1) * steps_for_each_thread,
              threads_number * steps_for_each_thread + init_steps_x % threads_number, std::ref(res));

  for (auto &worker: workers) worker.join();

  x_step /= 2;
  y_step /= 2;
  init_steps_x = init_steps_x << 1;
  init_steps_y = init_steps_y << 1;

  if (iteration == 0) return res;

  return res + prev_area / 4;
}

void Integral::rect_method(int iteration, size_t x_start_block, size_t x_end_block,
                           std::atomic<double> &gen_res) {
  double res = 0;

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = 0; j < init_steps_y; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      res += f_xy[func_to_int](x_start + i * x_step, y_start + j * y_step);
    }
  }
  gen_res += res * x_step * y_step;
}


std::pair<std::vector<double>, size_t> Integral::res_abserr_relerr_time(int func_number) {

  std::function<void(int, size_t, size_t, std::atomic<double> &)> rect_func;
  if (func_number == 1) {
    rect_func = [this](int iter, size_t a, size_t b, std::atomic<double>& res) {
    this->rect_method(iter, a, b, res);
    };
  }
  else {
    rect_func = [this](int iter, size_t a, size_t b, std::atomic<double>& res) {
    this->rect_method_atomic_bad(iter, a, b, res);
    };
  }

  auto start_time = get_current_time_fenced();

  double res = calc_function(0, 0, rect_func);

  double new_res = calc_function(1, res, rect_func);

  auto abs_diff_err = std::abs(new_res - res);

  auto rel_diff_err = boost::math::relative_difference(new_res, res);

  for (int i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res = new_res;

    new_res = calc_function(i, res, rect_func);

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}

///mutex, additional two

void Integral::rect_method_mutex(int iteration, size_t x_start_block,
                        size_t x_end_block, double &gen_res) {
  double res = 0;

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = 0; j < init_steps_y; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      res += f_xy[func_to_int](x_start + i * x_step, y_start + j * y_step);
    }
  }

  res *= x_step * y_step;

  mtx.lock();
  gen_res += res;
  mtx.unlock();
}

double Integral::calc_function_mutex(int iteration, double prev_area,
  std::function<void(int, size_t, size_t, double&)> rect_func) {
  double res = 0;

  std::vector<std::thread> workers(threads_number - 1);
  auto steps_for_each_thread = init_steps_x / threads_number;

  for (int i = 0; i < threads_number - 1; ++i) {
    workers[i] = std::thread([=, &res] {
        auto x_start_block = i * steps_for_each_thread;
        auto x_end_block = (i + 1) * steps_for_each_thread;
        rect_func(iteration, x_start_block, x_end_block, res);
      }
    );
  }

  rect_func(iteration, (threads_number - 1) * steps_for_each_thread,
              threads_number * steps_for_each_thread + init_steps_x % threads_number, res);

  for (auto &worker: workers) worker.join();

  x_step /= 2;
  y_step /= 2;
  init_steps_x = init_steps_x << 1;
  init_steps_y = init_steps_y << 1;

  if (iteration == 0) return res;

  return res + prev_area / 4;
}


std::pair<std::vector<double>, size_t> Integral::res_abserr_relerr_time_mutex(int func_number) {

  std::function<void(int, size_t, size_t, double &)> rect_func;
  if (func_number == 1) {
    rect_func = [this](int iter, size_t a, size_t b, double& res) {
    this->rect_method_mutex(iter, a, b, res);
    };
  }
  else {
    rect_func = [this](int iter, size_t a, size_t b, double& res) {
    this->rect_method_mutex_bad(iter, a, b, res);
    };
  }

  auto start_time = get_current_time_fenced();

  double res = calc_function_mutex(0, 0, rect_func);

  double new_res = calc_function_mutex(1, res, rect_func);

  auto abs_diff_err = std::abs(new_res - res);

  auto rel_diff_err = boost::math::relative_difference(new_res, res);

  for (int i = 2; (abs_diff_err > abs_err || rel_diff_err > rel_err) && i < max_iter; ++i) {
    res = new_res;

    new_res = calc_function_mutex(i, res, rect_func);

    abs_diff_err = abs(new_res - res);
    rel_diff_err = boost::math::relative_difference(new_res, res);
  }

  auto end_time = get_current_time_fenced();
  auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}

/// additional three

void Integral::rect_method_mutex_bad(int iteration, size_t x_start_block,
                        size_t x_end_block, double &gen_res) {

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = 0; j < init_steps_y; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      mtx.lock();
      gen_res += f_xy[func_to_int](x_start + i * x_step, y_start + j * y_step);
      mtx.unlock();
    }
  }
}

void Integral::rect_method_atomic_bad(int iteration, size_t x_start_block,
                        size_t x_end_block, std::atomic<double> &gen_res) {

  for (size_t i = x_start_block; i < x_end_block; ++i) {
    for (size_t j = 0; j < init_steps_y; ++j) {
      if (iteration && i % 2 == 0 && j % 2 == 0) continue;
      gen_res += f_xy[func_to_int](x_start + i * x_step, y_start + j * y_step);
    }
  }
}
