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
    cv.notify_all();
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
    cv.wait(lock, [&] { return !data.empty(); });

    popped_val = data.front();
    data.pop_front();
  }

  void move_dequeue(T &popped_val) {
    std::unique_lock lock(m);

    cv.wait(lock, [&] { return !data.empty(); });

    popped_val = std::move(data.front());
    data.pop_front();
  }

  void move_enqueue(T item) { {
      std::lock_guard lock(m);
      data.push_back(std::move(item));
    }
    cv.notify_all();
  }

  bool try_dequeue(T &item) {
    std::lock_guard lock(m);
    if (data.empty()) {
      return false;
    }
    item = std::move(data.front());
    data.pop_front();
    return true;
  }
};

#endif //THREAD_SAFE_QUEUE_H
