#ifndef THREAD_JOINER_H
#define THREAD_JOINER_H

#include <thread>
#include <vector>

// code taken from A.Williams, ”C++ concurrency in action”, 2nd edition

class thread_joiner {
  std::vector<std::thread> &threads;

public:
  explicit thread_joiner(std::vector<std::thread> &threads_);

  ~thread_joiner();
};


#endif //THREAD_JOINER_H
