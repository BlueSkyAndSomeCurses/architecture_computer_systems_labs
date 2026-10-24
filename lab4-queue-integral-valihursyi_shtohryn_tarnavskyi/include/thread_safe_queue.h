#pragma once
#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <deque>
#include <condition_variable>

template<class T>
class thread_safe_queue {
  std::deque<T> data;
  std::mutex m;
  std::condition_variable cv;

public:
  void enqueue(T item) { {
      std::lock_guard lock(m);
      data.push_back(item);
    }
    cv.notify_one();
  }

  bool empty() {
    std::lock_guard lock(m);
    return data.empty();
  }

  size_t size() {
    std::lock_guard lock(m);
    return data.size();
  }

  void dequeue(T &popped_val) {
    std::unique_lock lock(m);
    while (data.empty()) {
      cv.wait(lock);
    }

    popped_val = data.front();
    data.pop_front();
  }
};

#endif //THREAD_SAFE_QUEUE_H
