#ifndef CONCURRENT_HASHMAP_H
#define CONCURRENT_HASHMAP_H

#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <vector>
#include <deque>

// From here: https://blog.ycrash.io/optimizing-the-capacity-of-a-hashmap/
// default unordered_map uses 1, but we will need much more (stick to 2^n)
constexpr int DEFAULT_BUCKET_COUNT = 1024;
constexpr double LOAD_FACTOR = 0.75;


class ConcurrentHashMap {
  public:
    ConcurrentHashMap() : count_buckets(DEFAULT_BUCKET_COUNT), load_factor(LOAD_FACTOR) {
      data.resize(count_buckets);
      mutexes.resize(count_buckets);
    }

    int& operator[](const std::string& key) {
      std::shared_lock global_lock(global_mutex);
      size_t hash = std::hash<std::string>()(key);
      size_t index = hash % count_buckets;
      std::unique_lock<std::mutex> lock(mutexes[index]);
      for (auto& pair : data[index]) {
        if (pair.first == key) {
          return pair.second;
        }
      }
      ++count_elements;
      if (count_elements > count_buckets * load_factor) {
        lock.unlock();
        global_lock.unlock();
        rehash();
        return emplace_back_hashed(hash, key);
      }
      data[index].emplace_back(key, int());
      return data[index].back().second;
    }

    void get_all_elements(std::vector<std::pair<std::string, int>>& elements) const {
      for (int i = 0; i < count_buckets; ++i) {
        std::lock_guard<std::mutex> bucket_lock(mutexes[i]);
        elements.insert(elements.end(), data[i].begin(), data[i].end());
      }
    }

  private:
    int& emplace_back_hashed(size_t hash, const std::string& key) {
      std::shared_lock global_lock(global_mutex);
      size_t index = hash % count_buckets;
      std::lock_guard<std::mutex> lock(mutexes[index]);
      data[index].emplace_back(key, int());
      return data[index].back().second;
    }

    void rehash() {
      std::unique_lock global_lock(global_mutex);
      // if two threads started rehash at the same time, let only one to finish
      if (count_elements < count_buckets * load_factor) return;
      int new_count_buckets = count_buckets * 2;
      std::vector<Bucket> new_data(new_count_buckets);
      for (int i = 0; i < count_buckets; ++i) {
        std::lock_guard<std::mutex> lock(mutexes[i]);
        for (auto& pair : data[i]) {
          size_t hash = std::hash<std::string>()(pair.first);
          size_t new_index = hash % new_count_buckets;
          new_data[new_index].push_back(std::move(pair));
        }
      }
      mutexes.resize(new_count_buckets);
      data.swap(new_data);
      count_buckets = new_count_buckets;
    }

    typedef std::vector<std::pair<std::string, int>> Bucket;

    std::vector<Bucket> data;
    mutable std::deque<std::mutex> mutexes;
    std::atomic<int> count_elements = 0;
    int count_buckets;
    const double load_factor;
    std::shared_mutex global_mutex;
};


#endif //CONCURRENT_HASHMAP_H
