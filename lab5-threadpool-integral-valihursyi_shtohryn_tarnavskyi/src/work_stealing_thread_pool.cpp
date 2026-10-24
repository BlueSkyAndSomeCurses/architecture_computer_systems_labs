#include "work_stealing_thread_pool.h"

thread_local work_stealing_queue *work_stealing_thread_pool::local_work_queue = nullptr;
thread_local unsigned work_stealing_thread_pool::my_idx = 0;


void work_stealing_thread_pool::worker_thread(unsigned my_idx_) {
  my_idx = my_idx_;
  local_work_queue = queues[my_idx].get();
  while (!done) {
    run_pending_task();
  }
}

bool work_stealing_thread_pool::pop_task_from_local_queue(Task &task) {
  return local_work_queue && local_work_queue->try_pop(task);
}

bool work_stealing_thread_pool::pop_task_from_pool_queue(Task &task) {
  return pool_work_queue.try_dequeue(task);
}


bool work_stealing_thread_pool::pop_task_from_other_thread_queue(Task &task) const {
  for (unsigned i = 0; i < queues.size(); ++i) {
    unsigned const index = (my_idx + i + 1) % queues.size();

    if (queues[index]->try_steal(task)) {
      return true;
    }
  }
  return false;
}

work_stealing_thread_pool::work_stealing_thread_pool(size_t thread_number): done{false}, thread_joiner_{threads} {
  try {
    for (unsigned i = 0; i < thread_number; ++i) {
      queues.push_back(std::make_unique<work_stealing_queue>());
    }
    for (unsigned i = 0; i < thread_number; ++i) {
      threads.emplace_back(&work_stealing_thread_pool::worker_thread, this, i);
    }
  } catch (...) {
    done = true;
    throw;
  }
}

work_stealing_thread_pool::~work_stealing_thread_pool() {
  done = true;
}


std::future<double> work_stealing_thread_pool::submit(std::function<double(std::array<size_t, 5>)> input_func,
                                                      const std::array<size_t, 5> &args) {
  std::promise<double> prom;
  std::future<double> future = prom.get_future();

  Task task(std::move(input_func), args, std::move(prom));

  if (local_work_queue) {
    local_work_queue->push(std::move(task));
  } else {
    pool_work_queue.move_enqueue(std::move(task));
  }

  return future;
}

void work_stealing_thread_pool::run_pending_task() {
  Task task;
  if (pop_task_from_local_queue(task) ||
      pop_task_from_pool_queue(task) ||
      pop_task_from_other_thread_queue(task)) {
    double result = task.func(task.args);
    task.promise.set_value(result);
  }
}




