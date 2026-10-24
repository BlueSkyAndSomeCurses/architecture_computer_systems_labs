#include "timer.h"

size_t get_time(std::chrono::high_resolution_clock::time_point start,
                std::chrono::high_resolution_clock::time_point end) {
    return static_cast<size_t>(std::chrono::duration<double, std::milli>(
      end - start).
    count());
}
