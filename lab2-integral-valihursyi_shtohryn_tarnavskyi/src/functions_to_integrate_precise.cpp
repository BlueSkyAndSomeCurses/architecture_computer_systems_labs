#include "functions_to_integrate.h"
#include "precise_addition.h"
#include <cmath>

double langerman_a1[] = {1, 2, 1, 1, 5};
double langerman_a2[] = {4, 5, 1, 2, 4};
double langerman_c[] = {2, 1, 4, 7, 2};

double de_johnge_precise(double x, double y) {
    double f_x_y = 0.002;
    double c = 0;

    for (char i = -2; i <= 2; ++i) {

        for (char j = -2; j <= 2; ++j) {

            double d = 1 / (5 * (i + 2) + j + 3 + pow(x - 16 * j, 6) + pow(y - 16 * i, 6)) + c;
            auto [new_sum, t] = fast2Sum(f_x_y, d);
            f_x_y = new_sum;
            c = t;
        }

    }
    return 1 / f_x_y;
}


// No changes
double ackleys_function_precise(double x, double y) {
    return -20 * exp(-0.2 * std::sqrt(0.5 * (x * x + y * y))) - exp(
             0.5 * (cos(2 * std::numbers::pi * x) + cos(2 * std::numbers::pi * y))) + 20 + exp(1);
}

double langermann_function_precise(double x, double y) {
    double res = 0;
    double c = 0;

    for (int i = 0; i < 5; ++i) {

        double pow_x = pow(x - langerman_a1[i], 2);
        double pow_y = pow(y - langerman_a2[i], 2);

        double d = langerman_c[i] * exp(
          (-1 / std::numbers::pi)
          * (pow_x + pow_y)
        ) * cos(
          std::numbers::pi
          * (pow_x + pow_y)
        ) + c;

        auto [new_sum, t] = fast2Sum(res, d);
        res = new_sum;
        c = t;
    }

    return -res;
}