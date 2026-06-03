//
// Created by 66 on 2026/6/3.
//

#include "eventloop.h"

#include <unistd.h>
#include <sys/epoll.h>

#include "channel.h"


EventLoop::EventLoop() : epfd_(epoll_create1(0)), events_(1024) {
}

EventLoop::~EventLoop() {
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

void EventLoop::loop() {
    while (true) {
        int nfds = epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()), -1);

        for (int i = 0; i < nfds; i++) {
            auto *ch = static_cast<Channel *>(events_[i].data.ptr);
            ch->handleEvent();
        }
    }
}
