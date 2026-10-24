#include "integrate.h"

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
  int func_num, size_t thr_num, size_t prt_num
)
  : abs_err{abs_err}, rel_err{rel_err}, x_start{x_start}, x_end{x_end}, y_start{y_start}, y_end{y_end},
    init_steps_x{init_steps_x}, init_steps_y{init_steps_y}, max_iter{max_iter}, func_to_int{func_num - 1},
    threads_number{thr_num}, partition_number{prt_num}, thread_pool{thr_num} {
  x_step = (x_end - x_start) / init_steps_x;
  y_step = (y_end - y_start) / init_steps_y;
};

double Integral::calc_function(size_t iteration, double prev_area) {
  auto steps_for_each_thread_x = init_steps_x / threads_number;
  auto steps_for_each_thread_y = init_steps_y / partition_number;
  std::vector<std::future<double> > res_vec;

  std::function<double(std::array<size_t, 5>)> func =
      [this](std::array<size_t, 5> args) {
    return rect_method(args);
  };

  for (size_t i = 0; i < threads_number; ++i) {
    auto x_end_point = (i + 1) * steps_for_each_thread_x;
    if (i == threads_number - 1)
      x_end_point += init_steps_x % threads_number;
    for (size_t j = 0; j < partition_number; ++j) {
      auto y_end_point = (j + 1) * steps_for_each_thread_y;
      if (j == partition_number - 1)
        y_end_point += init_steps_y % partition_number;

      res_vec.emplace_back(thread_pool.submit(func, {
                                                iteration,
                                                i * steps_for_each_thread_x,
                                                x_end_point,
                                                j * steps_for_each_thread_y,
                                                y_end_point
                                              }));
    }
  }

  double res = 0;
  size_t i = 0;
  size_t it_numb = threads_number * partition_number;
  while (i < it_numb) {
    res += res_vec[i].get();
    ++i;
  }

  res *= x_step * y_step;

  x_step /= 2;
  y_step /= 2;
  init_steps_x = init_steps_x << 1;
  init_steps_y = init_steps_y << 1;

  return res + prev_area / 4;
}

double Integral::rect_method(const std::array<size_t, 5> &args) {
  double res = 0;
  size_t iteration = args[0];
  size_t x_start_block = args[1];
  size_t x_end_block = args[2];
  size_t y_start_block = args[3];
  size_t y_end_block = args[4];

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

  // std::function<double(std::array<size_t, 5>)> func;

#ifndef WORK_STEALING
  thread_pool.end_work();
#endif

  return {{new_res, abs_diff_err, rel_diff_err}, elapsed_time_ms};
}
