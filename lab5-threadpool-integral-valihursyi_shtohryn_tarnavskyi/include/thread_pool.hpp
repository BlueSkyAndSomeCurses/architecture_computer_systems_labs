#pragma once
#ifndef THREAD_SAFE
#define THREAD_SAFE

#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <functional>

#include "thread_safe_queue.h"
#include "task.hpp"

class ThreadPool {
private:
  std::vector<std::thread> workers;
  thread_safe_queue<Task> inputs;
  thread_safe_queue<double> outputs;

  void cont_calculator();

public:
  explicit ThreadPool(size_t thread_number);

  std::future<double> submit(std::function<double(std::array<size_t, 5>)> input_func,
                             const std::array<size_t, 5> &args);

  void end_work();
};

#endif
