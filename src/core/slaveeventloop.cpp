//
// Created by 66 on 2026/6/5.
//

#include "slaveeventloop.h"

SlaveEventLoop::SlaveEventLoop()
    : EventLoop() {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    struct itimerspec ts{};
    ts.it_value.tv_sec = 1;
    ts.it_interval.tv_sec = 1;
    timerfd_settime(timer_fd, 0, &ts, nullptr);

    timer_channel_ = new Channel(timer_fd, EPOLLIN);
    timer_channel_->setReadCallBack([this]() {
        handleTimeout();
    });

    addChannel(timer_channel_);
}

SlaveEventLoop::~SlaveEventLoop() {
    delete timer_channel_;
}

void SlaveEventLoop::closeAllConnection() {
    auto tmp = connections_;
    for (auto &conn: tmp) {
        conn->mClose();
    }
}

void SlaveEventLoop::handleTimeout() {
    uint64_t expriations;
    read(timer_channel_->fd(), &expriations, sizeof(expriations));

    auto now = std::chrono::steady_clock::now();
    int timeout = timeout_seconds_.load(std::memory_order_relaxed);

    auto tmp = connections_;
    for (auto *conn: tmp) {
        if (now - conn->lastActive() > std::chrono::seconds(timeout)) {
            std::cout << "Connection " << conn->fd()
                    << " timeout after " << timeout << "s, closing"
                    << std::endl;
            conn->mClose();
        }
    }
}
