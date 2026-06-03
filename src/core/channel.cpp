//
// Created by 66 on 2026/6/3.
//

#include "channel.h"

#include <sys/epoll.h>

Channel::Channel(int fd, uint32_t events) : fd_(fd), events_(events) {
}

void Channel::handleEvent() const {
    if (events_ & EPOLLIN) {
        if (read_cb_) read_cb_();
    }
}
