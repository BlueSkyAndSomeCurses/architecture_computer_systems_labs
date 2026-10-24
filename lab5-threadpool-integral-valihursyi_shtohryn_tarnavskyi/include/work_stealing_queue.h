
#ifndef WORK_STEALLING_QUEUE_H
#define WORK_STEALLING_QUEUE_H

#include "task.hpp"
#include <deque>

class work_stealing_queue {
  std::deque<Task> queue;
  mutable std::mutex mutex;
  public:
  work_stealing_queue();
  work_stealing_queue(const work_stealing_queue& other) = delete;
  work_stealing_queue& operator=(const work_stealing_queue& other) = delete;

  void push(Task task);
  bool empty() const;
  bool try_pop(Task& res);
  bool try_steal(Task& res);
};



#endif //WORK_STEALLING_QUEUE_H
