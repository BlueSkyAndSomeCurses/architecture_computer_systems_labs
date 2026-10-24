#ifndef WORK_STEALING_THREAD_POOL_H
#define WORK_STEALING_THREAD_POOL_H

#include <atomic>
#include <thread_safe_queue.h>
#include "task.hpp"
#include "work_stealing_queue.h"
#include "thread_joiner.h"

class work_stealing_thread_pool {
  std::atomic<bool> done;
  thread_safe_queue<Task> pool_work_queue;
  std::vector<std::unique_ptr<work_stealing_queue> > queues;
  std::vector<std::thread> threads;
  thread_joiner thread_joiner_;

  static thread_local work_stealing_queue *local_work_queue;

  static thread_local unsigned my_idx;

  void worker_thread(unsigned my_idx_);

  static bool pop_task_from_local_queue(Task &task);

  bool pop_task_from_pool_queue(Task &task);

  bool pop_task_from_other_thread_queue(Task &task) const;

public:
  explicit work_stealing_thread_pool(size_t thread_number);

  ~work_stealing_thread_pool();


  std::future<double> submit(std::function<double(std::array<size_t, 5>)> input_func,
                             const std::array<size_t, 5> &args);


  void run_pending_task();
};


#endif //WORK_STEALING_THREAD_POOL_H
