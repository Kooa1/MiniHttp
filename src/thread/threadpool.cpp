//
// Created by 66 on 2026/6/3.
//

#include "threadpool.h"

ThreadPool::ThreadPool(size_t thread_count) {
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this]() { workerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    mStop();
}

void ThreadPool::mStop() { {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        stop_ = true;
    }

    cv_.notify_all();
    for (auto &worker: workers_) {
        if (worker.joinable()) {
            worker.joinable();
        }
    }
}

void ThreadPool::submit(Task task) { {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        Task task; {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {
                return stop_ || tasks_.empty();
            });

            if (stop_ && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}
