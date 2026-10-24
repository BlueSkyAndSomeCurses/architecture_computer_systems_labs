#ifndef INTEGRATE_PRECISE_H
#define INTEGRATE_PRECISE_H

#include <functional>
#include <vector>
#include "functions_to_integrate_precise.h"

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
    std::vector<std::function<double(double, double)> > f_xy = {de_johnge_precise, ackleys_function_precise, langermann_function_precise};

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

      int max_iter, int func_num
    );

    double calc_function(int iteration, double prev_area);

    double rect_method_precise(int iteration);

    std::pair<std::vector<double>, size_t> res_abserr_relerr_time();
};

#endif //INTEGRATE_PRECISE_H


