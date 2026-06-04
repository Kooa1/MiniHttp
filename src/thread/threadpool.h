//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_THREADPOOL_H
#define MINIHTTP_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>


class ThreadPool {
public:
    using Task = std::function<void()>;

    explicit ThreadPool(size_t thread_count);

    ~ThreadPool();

    void submit(Task task);

    void mStop();

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};


#endif //MINIHTTP_THREADPOOL_H
