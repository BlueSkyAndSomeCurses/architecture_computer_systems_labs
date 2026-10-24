#include <iostream>
#include <gsl/gsl_integration.h>

#include "check_file_reading.h"
#include "toml.hpp"
#include "options_parser.h"
#include "timer.h"
#include "functions_to_integrate.h"

struct integration_params {
    double x;
    double c;
    double d;
    double (*f)(double, double);
    int partition_inner;
    double abs_err_inner;
    double rel_err_inner;
};


double inner_integrand(double y, void *params) {
    struct integration_params *p = (struct integration_params *) params;
    return p->f(p->x, y);
}

double outer_integrand(double x, void *params) {
    struct integration_params *op = (struct integration_params *) params;

    struct integration_params ip = {
        .x = x,
        .c = op->c,
        .d = op->d,
        .f = op->f,
        .partition_inner = op->partition_inner,
        .abs_err_inner = op->abs_err_inner,
        .rel_err_inner = op->rel_err_inner
    };

    gsl_function F;
    F.function = &inner_integrand;
    F.params = &ip;

    gsl_integration_workspace *workspace = gsl_integration_workspace_alloc(op->partition_inner);
    double result, error;

    gsl_integration_qag(&F, ip.c, ip.d, ip.abs_err_inner, ip.rel_err_inner, op->partition_inner, 
                       GSL_INTEG_GAUSS15, workspace, &result, &error);
    
    gsl_integration_workspace_free(workspace);
    return result;
}

std::pair<double, double> gsl_double_integral(double (*f)(double, double), 
                          double a, double b, 
                          double c, double d,
                          int partition, double abs_err,
                          double rel_err) {
    gsl_integration_workspace *workspace = gsl_integration_workspace_alloc(partition);
    
    struct integration_params params = {
        .x = 0,
        .c = c,
        .d = d,
        .f = f,
        .partition_inner = partition,
        .abs_err_inner = abs_err,
        .rel_err_inner = rel_err
    };

    gsl_function F;
    F.function = &outer_integrand;
    F.params = &params;

    double result, error;
    gsl_integration_qag(&F, a, b, abs_err, rel_err, partition, 
                       GSL_INTEG_GAUSS15, workspace, &result, &error);

    gsl_integration_workspace_free(workspace);
    return {result, error};
}

int main(int argc, char* argv[]) {
    auto [method_num, file_name] = check_file_reading(argc, argv);

    toml::table tbl;
    try {
        tbl = toml::parse_file(file_name);
    } catch (const toml::parse_error &err) {
        std::cerr << "помилка читання файлу конфігурації, або помилка його вмісту" << err << "\n";
        std::exit(5);
    }

    auto abs_err = tbl["abs_err"].value<double>().value();
    auto rel_err = tbl["rel_err"].value<double>().value();
    auto x_start = tbl["x_start"].value<double>().value();
    auto x_end = tbl["x_end"].value<double>().value();
    auto y_start = tbl["y_start"].value<double>().value();
    auto y_end = tbl["y_end"].value<double>().value();
    auto init_steps_x = tbl["init_steps_x"].value<double>().value();
    auto init_steps_y = tbl["init_steps_y"].value<double>().value();
    auto max_iter = tbl["max_iter"].value<int>().value();

    
    std::vector<double (*)(double, double)> f_xy = {
        de_johnge, 
        ackleys_function, 
        langermann_function
    };

    auto choosen_f = f_xy[method_num - 1];

    auto start_time = get_current_time_fenced();

    auto partition = init_steps_x * init_steps_y;

    auto [res, new_abs_err] = gsl_double_integral(
        choosen_f, x_start, x_end, y_start,
        y_end, partition, abs_err, rel_err
    );

    double new_rel_err = new_abs_err / std::abs(res);

    for (int i = 1; i < max_iter && (new_abs_err > abs_err || new_rel_err > rel_err); i++) {

        partition *= 4;

        auto current_result = gsl_double_integral(
            choosen_f, x_start, x_end, y_start,
            y_end, partition, abs_err, rel_err
        );

        res = current_result.first;
        new_abs_err = current_result.second;
        new_rel_err = new_abs_err / std::abs(res);
    }

    auto end_time = get_current_time_fenced();

    auto elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end_time - start_time).
    count());

    std::cout << res << std::endl;
    std::cout << new_abs_err << std::endl;
    std::cout << new_rel_err << std::endl;
    std::cout << elapsed_time_ms << std::endl;

    return 0;
}
