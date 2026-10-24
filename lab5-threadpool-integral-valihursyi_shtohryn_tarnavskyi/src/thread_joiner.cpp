#include "thread_joiner.h"

// code taken from A.Williams, ”C++ concurrency in action”, 2nd edition
thread_joiner::thread_joiner(std::vector<std::thread> &threads_): threads{threads_} {
}

thread_joiner::~thread_joiner() {
  for (auto &thread: threads) { if (thread.joinable()) thread.join(); }
}


