//
// Created by 66 on 2026/6/5.
//

#include "eventloopgroup.h"

EventLoopGroup::EventLoopGroup(size_t slave_count) {
    slaves_.reserve(slave_count);
    for (size_t i = 0; i < slave_count; ++i) {
        slaves_.push_back(std::make_unique<SlaveEventLoop>());
    }
}

EventLoopGroup::~EventLoopGroup() {
    stopAll();
}

void EventLoopGroup::mStart() {
    threads_.reserve(slaves_.size());
    for (auto &slave: slaves_) {
        threads_.emplace_back([slave = slave.get()]() {
            slave->loop();
        });
    }
}

SlaveEventLoop *EventLoopGroup::next() {
    size_t idx = next_.fetch_add(1, std::memory_order_relaxed) % slaves_.size();
    return slaves_[idx].get();
}

void EventLoopGroup::stopAll() {
    for (auto &slave: slaves_) {
        slave->mStop();
    }

    for (auto &t: threads_) {
        if (t.joinable()) {
            t.join();
        }
    }

    for (auto &slave: slaves_) {
        slave->closeAllConnection();
    }
}
