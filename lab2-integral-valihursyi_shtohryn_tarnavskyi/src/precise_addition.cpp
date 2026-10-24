#include <cmath>

#include "precise_addition.h"

std::pair<double, double> fast2Sum(double const a, double const b) {
    double s = a + b;
    double t = b - (s - a);

    return {s, t};
}