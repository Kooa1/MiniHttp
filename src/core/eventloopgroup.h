//
// Created by 66 on 2026/6/5.
//

#ifndef MINIHTTP_EVENTLOOPGROUP_H
#define MINIHTTP_EVENTLOOPGROUP_H

#include <atomic>
#include <memory>
#include <vector>
#include <thread>

#include "slaveeventloop.h"

class EventLoopGroup {
public:
    explicit EventLoopGroup(size_t slave_count);

    ~EventLoopGroup();

    void mStart();

    SlaveEventLoop *next();

    void stopAll();

    void setTimeout(int seconds);

private:
    std::vector<std::unique_ptr<SlaveEventLoop> > slaves_;
    std::vector<std::thread> threads_;
    std::atomic<size_t> next_{0};
};


#endif //MINIHTTP_EVENTLOOPGROUP_H
