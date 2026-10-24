#include <iostream>
#include <cmath>

#include "additional_integral.h"
#include "precise_addition.h"

double additional_function_part(double x) {

    double sum = 0.0;
    double c = 0.0;

    for (int i = 1; i < 6; i++) {

      double d = i * std::cos((i+1) * x + 1) + c;
      auto [new_sum, t] = fast2Sum(sum, d);
      sum = new_sum;
      c = t;
    }

    return sum;
}