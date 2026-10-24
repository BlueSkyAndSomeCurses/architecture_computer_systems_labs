//
// Created by vitya on 13.04.2025.
//

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class thread_safe_queue {
public:
  thread_safe_queue(size_t max_size) : max_queue_size(max_size) {
  }

  int size() {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.size();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
  }

  void push(const T &value) {
    std::unique_lock<std::mutex> lock(mutex);
    cv_size.wait(lock, [this] { return queue.size() < max_queue_size; });
    queue.push(value);
    lock.unlock();
    cv.notify_one();
  }

  void move_push(T &&value) {
    std::unique_lock<std::mutex> lock(mutex);
    cv_size.wait(lock, [this] { return queue.size() < max_queue_size; });
    queue.push(std::move(value));
    lock.unlock();
    cv.notify_one();
  }

  void pop(T &new_value) {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !queue.empty(); });
    new_value = queue.front();
    queue.pop();
    lock.unlock();
    cv_size.notify_one();
  }

  T move_pop() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !queue.empty(); });
    T value = std::move(queue.front());
    queue.pop();
    lock.unlock();
    cv_size.notify_one();
    return value;
  }

private:
  const size_t max_queue_size;
  std::queue<T> queue;
  std::mutex mutex;
  std::condition_variable cv;
  std::condition_variable cv_size;
};

#endif //THREAD_SAFE_QUEUE_H
