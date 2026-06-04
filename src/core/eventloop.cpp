//
// Created by 66 on 2026/6/3.
//

#include "eventloop.h"

EventLoop::EventLoop()
    : epfd_(epoll_create1(0)),
      events_(1024),
      eventfd_(Socket(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))),
      wakeup_channel_(new Channel(eventfd_.get(), EPOLLIN)) {
    wakeup_channel_->setReadCallBack([this]() {
        uint64_t one;
        read(eventfd_.get(), &one, sizeof(one));
    });
    addChannel(wakeup_channel_);
}

EventLoop::~EventLoop() {
    removeChannel(wakeup_channel_);
    delete wakeup_channel_;
    close(epfd_);
}

void EventLoop::addChannel(Channel *ch) {
    struct epoll_event ev{};
    ev.events = ch->events();
    ev.data.ptr = ch;
    epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->fd(), &ev);
}

void EventLoop::removeChannel(Channel *ch) {
    epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
}

void EventLoop::updateChannel(Channel *ch) {
    struct epoll_event ev{};
    ev.events = ch->events();
    ev.data.ptr = ch;
    epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->fd(), &ev);
}

void EventLoop::queueInLoop(Functor function) { {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        pending_functors_.push_back(std::move(function));
    }
    uint64_t one = 1;
    write(eventfd_.get(), &one, sizeof(one));
}

void EventLoop::runInLoop(Functor function) {
    if (isInLoopThread()) {
        function();
    } else {
        queueInLoop(std::move(function));
    }
}

bool EventLoop::isInLoopThread() const {
    return std::this_thread::get_id() == thread_id;
}

void EventLoop::loop() {
    thread_id = std::this_thread::get_id();
    while (!quit_) {
        int nfds = epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()), -1);

        for (int i = 0; i < nfds; i++) {
            auto *ch = static_cast<Channel *>(events_[i].data.ptr);
            ch->handleEvent();
        }
        doPendingFunctor();
    }
}

void EventLoop::mStop() {
    quit_ = true;
    uint64_t one = 1;
    write(eventfd_.get(), &one, sizeof(one));
}

void EventLoop::doPendingFunctor() {
    std::vector<Functor> functors; {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        functors.swap(pending_functors_);
    }
    for (auto &f: functors) {
        f();
    }
}
