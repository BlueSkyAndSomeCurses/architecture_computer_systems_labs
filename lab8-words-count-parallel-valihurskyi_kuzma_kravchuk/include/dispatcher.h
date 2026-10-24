#include <iostream>
#include <filesystem>
#include <boost/locale.hpp>
#include <barrier>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "config_parser.h"
#include "thread_safe_queue.h"
#include "word_counter.h"

#ifndef DISPATCHER_H
#define DISPATCHER_H

void process_text_with_boost(const std::string &text, word_count_t &word_count) {
    boost::locale::boundary::ssegment_index word_index(
      boost::locale::boundary::word, text.begin(), text.end());
    word_index.rule(boost::locale::boundary::word_letter);

    for (auto it = word_index.begin(); it != word_index.end(); ++it) {
        std::string word = *it;
        word = boost::locale::fold_case(boost::locale::normalize(word));
        ++word_count[word];
    }
}

    class DynamicDispatcher {
private:
    Config& config;
    ThreadPool pool;
    thread_safe_queue<std::pair<std::string, char>>& raw_files_queue;
    thread_safe_queue<word_count_t>& dictionaries_queue;

    std::atomic<bool> shutdown_requested{false};
    std::atomic<size_t> active_indexing_tasks{0};
    std::atomic<size_t> active_merging_tasks{0};
    std::atomic<size_t> remaining_files{0};

    std::mutex dispatch_mutex;
    std::condition_variable dispatch_cv;

    std::thread adjuster;

public:

    DynamicDispatcher(Config& cfg,
                    thread_safe_queue<std::pair<std::string, char>>& raw_queue,
                    thread_safe_queue<word_count_t>& dict_queue)
        : config(cfg),
          pool(cfg.max_total_threads),
          raw_files_queue(raw_queue),
          dictionaries_queue(dict_queue) {}

    void start() {
        for (size_t i = 0; i < static_cast<size_t>(config.min_indexing_threads); ++i)
            spawn_indexing_worker();
        for (size_t i = 0; i < static_cast<size_t>(config.min_merging_threads); ++i)
            spawn_merging_worker();

        adjuster = std::thread(&DynamicDispatcher::dynamic_adjustment_loop, this);
    }


    void shutdown() {
        shutdown_requested = true;
        dispatch_cv.notify_all();

        for (size_t i = 0; i < static_cast<size_t>(config.max_total_threads); ++i){
            raw_files_queue.push({"", -1});
        }

        if (adjuster.joinable())
            adjuster.join();
    }


    void set_total_files(size_t count) {
        remaining_files = count;
    }

    void set_indexing_latch(std::latch& latch) {
        indexing_latch_ptr = &latch;
    }


private:
    std::latch* indexing_latch_ptr = nullptr;

    void dynamic_adjustment_loop() {
        while (!shutdown_requested) {
            std::unique_lock<std::mutex> lock(dispatch_mutex);
            dispatch_cv.wait_for(
                lock,
                std::chrono::milliseconds(config.adjust_interval_ms),
                [&]() { return shutdown_requested.load(); }
            );

            if (shutdown_requested) break;

            size_t raw_size = raw_files_queue.size();
            size_t dict_size = dictionaries_queue.size();
            size_t total_active = active_indexing_tasks + active_merging_tasks;

            size_t desired_indexing = config.min_indexing_threads;
            size_t desired_merging = config.min_merging_threads;

            if (raw_size > dict_size * 2) {
                desired_indexing = std::min<size_t>(
                    config.max_indexing_threads,
                    desired_indexing + (raw_size / 10)
                );
            }
            else if (dict_size > raw_size * 2) {
                desired_merging = std::min<size_t>(
                    config.max_merging_threads,
                    desired_merging + (dict_size / 10)
                );
            }

            size_t total_desired = desired_indexing + desired_merging;
            if (total_desired > static_cast<size_t>(config.max_total_threads)){
                float ratio = static_cast<float>(config.max_total_threads) / total_desired;
                desired_indexing = static_cast<size_t>(desired_indexing * ratio);
                desired_merging = config.max_total_threads - desired_indexing;
            }

            adjust_workers(desired_indexing, desired_merging);
        }
    }

    void adjust_workers(size_t desired_indexing, size_t desired_merging) {
        while (active_indexing_tasks < desired_indexing &&
       (active_indexing_tasks + active_merging_tasks) < static_cast<size_t>(config.max_total_threads)) {
            spawn_indexing_worker();
        }

        while (active_merging_tasks < desired_merging &&
       (active_indexing_tasks + active_merging_tasks) < static_cast<size_t>(config.max_total_threads)){
            spawn_merging_worker();
        }
    }

    void spawn_indexing_worker() {
        pool.enqueue([this]() {
            active_indexing_tasks++;
            indexing_worker_task();
            active_indexing_tasks--;
        });
    }

    void spawn_merging_worker() {
        pool.enqueue([this]() {
            active_merging_tasks++;
            merging_worker_task();
            active_merging_tasks--;
        });
    }

    void indexing_worker_task() {
        while (!shutdown_requested) {
            auto [content, is_file] = raw_files_queue.move_pop();

            if (is_file == -1) {
                if (indexing_latch_ptr) {
                    indexing_latch_ptr->count_down();
                }
                return;
            }

            word_count_t word_count;
            static thread_local Archivator archivator;

            if (is_file) {
                process_text_with_boost(content, word_count);
            } else {
                archivator.process_content(content, config, word_count);
            }

            if (!word_count.empty()) {
                dictionaries_queue.move_push(std::move(word_count));
                dispatch_cv.notify_one();
            }
        }
    }

    void merging_worker_task() {
        while (!shutdown_requested) {
            word_count_t merged_result;

            if (dictionaries_queue.size() < 2) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            auto first = dictionaries_queue.move_pop();
            auto second = dictionaries_queue.move_pop();

            for (auto& [word, count] : second) {
                first[word] += count;
            }

            dictionaries_queue.move_push(std::move(first));
            dispatch_cv.notify_one();
        }
    }
};


#endif //DISPATCHER_H
