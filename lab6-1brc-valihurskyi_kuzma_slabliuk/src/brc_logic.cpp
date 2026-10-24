#include "brc_logic.h"

#include <algorithm>

#include "timer.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <ranges>
#include <utility>

constexpr size_t LINES_PER_TASK = 1000;
// 100 bytes (line size) * 1000 * 1000 = 100 MB
constexpr size_t MAX_QUEUE_SIZE = 1000;


float StationObservations::min() const {
  return static_cast<float>(min_obs) / 10;
}

float StationObservations::max() const {
  return static_cast<float>(max_obs) / 10;
}


void StationObservations::update(const int new_obs, std::string name) {
  if (st_name.empty()) st_name = std::move(name);


  ++observations_count;
  sum += new_obs;
  min_obs = min_obs < new_obs ? min_obs : new_obs;
  max_obs = max_obs > new_obs ? max_obs : new_obs;
}

void StationObservations::merge(const StationObservations &other) {
  if (st_name.empty()) st_name = other.st_name;

  observations_count += other.observations_count;
  sum += other.sum;
  min_obs = std::min(min_obs, other.min_obs);
  max_obs = std::max(max_obs, other.max_obs);
}

double StationObservations::mean() const {
  return observations_count > 0
           ? static_cast<float>(sum)
             / (10 * static_cast<float>(observations_count))
           : 0.0;
}


void process_part(ThreadSafeQueue<std::vector<std::string> > &queue,
                  ThreadSafeQueue<std::unordered_map<std::string, StationObservations> > &observations_queue) {
  while (true) {
    auto lines = queue.pop();
    if (lines.empty()) {
      break;
    }
    std::unordered_map<std::string, StationObservations> observations;
    for (auto &line: lines) {
      auto after_coma = line[line.size() - 1] - '0';
      line.erase(line.size() - 2, 3);

      auto semicolon_pos = line.rfind(';');
      auto name = line.substr(0, semicolon_pos),
          value = line.substr(semicolon_pos + 1);
      if (value[0] == '-')after_coma = -after_coma;
      observations[name].update(atoi(value.c_str()) * 10 + after_coma, name);
    }
    observations_queue.push(std::move(observations));
  }
}

void merge_observations(
  ThreadSafeQueue<std::unordered_map<std::string, StationObservations> > &
  observations_queue,
  std::unordered_map<std::string, StationObservations> &observations) {
  while (true) {
    auto observations_part = observations_queue.pop();
    if (observations_part.empty()) {
      break;
    }
    for (auto &it: observations_part) {
      observations[it.second.st_name].merge(it.second);
    }
  }
}

size_t process_observations(
  std::unordered_map<std::string, StationObservations> &observations,
  std::string &file_name,
  int num_threads) {
  ThreadSafeQueue<std::vector<std::string> > queue{MAX_QUEUE_SIZE};
  ThreadSafeQueue<std::unordered_map<std::string, StationObservations> >
      observations_queue{MAX_QUEUE_SIZE};

  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(process_part, std::ref(queue), std::ref(observations_queue));
  }
  std::thread merge_thread(merge_observations, std::ref(observations_queue), std::ref(observations));

  size_t curr_vector_size = 0;
  size_t total_tasks_sent = 0;
  std::vector<std::string> lines;
  lines.reserve(LINES_PER_TASK);

  size_t elapsed_time_ms = 0;
  std::chrono::high_resolution_clock::time_point stage_one_time = get_current_time_fenced();
  std::chrono::high_resolution_clock::time_point stage_two_time;

  std::ifstream file(file_name);

  while (file.good()) {
    lines.emplace_back();
    getline(file, lines.back());
    ++curr_vector_size;
    if (curr_vector_size == LINES_PER_TASK || file.eof()) {
      // update reading time
      // stage_two_time = get_current_time_fenced();
      // elapsed_time_ms += static_cast<size_t>(std::chrono::duration<double, std::milli>(
      // stage_two_time - stage_one_time).count());
      // push to queue
      if (lines.back().empty()) {
        lines.pop_back();
      }
      if (lines.empty()) {
        continue;
      }
      queue.push(std::move(lines));
      curr_vector_size = 0;
      ++total_tasks_sent;
      lines.clear();
      lines.reserve(LINES_PER_TASK);
    }
  }

  for (int i = 0; i < num_threads; ++i) {
    queue.push(std::vector<std::string>()); // poison pill
  }

  // Wait for all worker threads to finish
  for (auto &thread: threads) { thread.join(); }
  observations_queue.push(std::unordered_map<std::string, StationObservations>());
  // poison pill
  merge_thread.join();

  stage_two_time = get_current_time_fenced();

  elapsed_time_ms += static_cast<size_t>(std::chrono::duration<double, std::milli>(
    stage_two_time - stage_one_time).count());

  return elapsed_time_ms;
}


size_t write_results(
  std::unordered_map<std::string, StationObservations> &observations,
  std::string &outfile) {
  std::chrono::high_resolution_clock::time_point stage_one_time = get_current_time_fenced();
  std::ofstream file(outfile);


  auto stations = observations | std::ranges::views::keys | std::ranges::to<std::vector>();


  std::sort(stations.begin(), stations.end());

  for (auto &station: stations) {
    auto &station_obs = observations[station];
    std::string res = std::string(station) + "="
                      + std::format("{:.1f}", station_obs.min())
                      + "/" + std::format("{:.1f}",
                                          std::round(station_obs.mean() * 10) / 10) + "/"
                      + std::format("{:.1f}",
                                    station_obs.max()) + '\n';
    file << res;
  }

  std::chrono::high_resolution_clock::time_point stage_two_time = get_current_time_fenced();

  size_t elapsed_time_ms = static_cast<size_t>(std::chrono::duration<double, std::milli>(
      stage_two_time - stage_one_time).
    count());

  return elapsed_time_ms;
}
