#ifndef STATION_OBSERVATIONS_H
#define STATION_OBSERVATIONS_H
#include <unordered_map>
#include <string>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>


template<typename T>
class ThreadSafeQueue {
public:
  ThreadSafeQueue(size_t max_size) : max_queue_size(max_size) {
  }

  void push(const T &&value) {
    std::unique_lock<std::mutex> lock(mutex);
    cv_size.wait(lock, [this] { return queue.size() < max_queue_size; });
    queue.push(std::move(value));
    lock.unlock();
    cv.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !queue.empty(); });
    T value = std::move(queue.front());
    if (value.empty()) { return value; } // it's a poison pill, don't pop it
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

class StationObservations {
  int observations_count = 0;

  long sum = 0;
  long min_obs = 1100;
  long max_obs = -1100;

public:
  std::string st_name;

  [[nodiscard]] float min() const;

  [[nodiscard]] float max() const;

  StationObservations() = default;

  void update(int new_obs, std::string name);

  void merge(const StationObservations &other);

  [[nodiscard]] double mean() const;
};

size_t process_observations(
  std::unordered_map<std::string, StationObservations> &observations,
  std::string &file_name,
  int num_threads);

void process_part(ThreadSafeQueue<std::vector<std::string> > &queue,
                  ThreadSafeQueue<std::unordered_map<std::string, StationObservations> > &observations_queue);

void merge_observations(
  ThreadSafeQueue<std::unordered_map<std::string, StationObservations> > &
  observations_queue,
  std::unordered_map<std::string, StationObservations> &observations);

size_t write_results(
  std::unordered_map<std::string, StationObservations> &observations,
  std::string &file_name);


#endif //STATION_OBSERVATIONS_H
