#ifndef ALTERNATIVE_METHOD_H
#define ALTERNATIVE_METHOD_H

#include <functional>
#include <vector>
#include "functions_to_integrate.h"
#include <thread>
#include <thread_safe_queue.h>

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
    int partition_number;

    std::atomic<int> expected_num;

    thread_safe_queue<std::vector<size_t> > inputs;
    thread_safe_queue<std::vector<double>> outputs;
    std::vector<std::function<double(double, double)> > f_xy = {de_johnge, ackleys_function, langermann_function};
    std::vector<std::thread> workers;

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

      int max_iter, int func_num, int thr_num, int prt_num
    );


    std::vector<double> calc_function();

    void cont_calculator();

    double rect_method(size_t iteration, size_t x_start_block, size_t x_end_block, size_t y_start_block,
                       size_t y_end_block, double cur_x_step, double cur_y_step);

    std::pair<std::vector<double>, size_t> res_abserr_relerr_time();
};

class Interval {
    size_t cur_x_start;
    size_t cur_y_start;

    double cur_x_step;
    double cur_y_step;

    size_t x_num;
    size_t y_num;

public:
    Interval(size_t cur_x_start, size_t cur_y_start, double cur_x_step, double cur_y_step, size_t x_num, size_t y_num);

};


#endif //ALTERNATIVE_METHOD_H
