#include "work_stealing_queue.h"

work_stealing_queue::work_stealing_queue() = default;

void work_stealing_queue::push(Task task) {
  std::lock_guard<std::mutex> lock(mutex);
  queue.push_front(std::move(task));
}

bool work_stealing_queue::empty () const {
  std::lock_guard<std::mutex> lock(mutex);
  return queue.empty();
}

bool work_stealing_queue::try_pop(Task& res) {
  std::lock_guard<std::mutex> lock(mutex);
  if (queue.empty()) {
    return false;
  }
  res = std::move(queue.front());
  queue.pop_front();
  return true;
}

bool work_stealing_queue::try_steal(Task &res) {
  std::lock_guard<std::mutex> lock(mutex);
  if (queue.empty()) {
    return false;
  }
  res = std::move(queue.back());
  queue.pop_back();
  return true;
}


