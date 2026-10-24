#ifndef INTEGRATE_H
#define INTEGRATE_H

#include <functional>
#include <vector>
#include <random>
#include <gsl/gsl_integration.h>
#include "functions_to_integrate.h"

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
  std::vector<std::function<double(double, double)> > f_xy = {de_johnge, ackleys_function, langermann_function};

  std::mt19937 gen;
  std::uniform_real_distribution<double> dist_x;
  std::uniform_real_distribution<double> dist_y;
  double monte_area;
  std::function<double(double, double)> current_func;

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

  double rect_method(int iteration);

  std::pair<std::vector<double>, size_t> res_abserr_relerr_time();

  std::pair<std::vector<double>, size_t> monte_carlo_time();

  double monte_carlo_calc(int samples, int previous_samples, double prev_integ);

  double monte_carlo(int n_samples);

  std::pair<double, double> additional_calc(double x_step, double y_step, int iteration, double prev_area_x, double prev_area_y);
  double calc_function_additional(double step, double start, double end, int iteration, double prev_area);
  double rect_method_additional(double step, double new_start, double end);
  std::pair<std::vector<double>, size_t> additional_function_results();
};


#endif //INTEGRATE_H
