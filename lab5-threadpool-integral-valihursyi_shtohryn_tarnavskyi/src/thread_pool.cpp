#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t threads_number) {
  workers.resize(threads_number);

  for (size_t i = 0; i < threads_number; i++) {
    workers[i] = std::thread([&] { cont_calculator(); });
  }
}

std::future<double> ThreadPool::submit(std::function<double(std::array<size_t, 5>)> input_func,
                                       const std::array<size_t, 5> &args) {
  std::promise<double> prom;
  std::future<double> fut = prom.get_future();

  Task task(std::move(input_func), args, std::move(prom));

  inputs.move_enqueue(std::move(task));

  return fut;
}

void ThreadPool::cont_calculator() {
  while (true) {
    Task task;
    inputs.move_dequeue(task);

    if (task.args[1] > task.args[2]) {
      return;
    }

    double result = task.func(task.args);
    task.promise.set_value(result);
  }
}

void ThreadPool::end_work() {
  size_t threads_number = workers.size();
  const std::function func = [](std::array<size_t, 5>) { return 0.0; };

  for (size_t i = 0; i < threads_number; ++i)
    submit(func, {0, 1, 0, 1, 0});
  for (size_t i = 0; i < threads_number; ++i)
    workers[i].join();
}


