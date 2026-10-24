#ifndef DYNAMIC_THREAD_POOL_H
#define DYNAMIC_THREAD_POOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <cstddef>
#include <iostream>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> stop;
    std::function<void()> task;

public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> local_task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->cv.wait(lock, [this] {
                            return this->stop.load() || this->task != nullptr;
                        });
                        if(this->stop && this->task == nullptr) return;
                        local_task = std::move(this->task);
                        this->task = nullptr;
                    }
                    local_task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            task = std::forward<F>(f);
        }
        cv.notify_one();
    }

    ~ThreadPool() {
        stop = true;
        cv.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }
};


#endif // DYNAMIC_THREAD_POOL_H
