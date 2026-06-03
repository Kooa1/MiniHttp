//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_EVENTLOOP_H
#define MINIHTTP_EVENTLOOP_H

#include <vector>

#include "core/channel.h"

class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    void addChannel(Channel *ch);

    void removeChannel(Channel *ch);

    void updateChannel(Channel *ch);

    void loop();

private:
    int epfd_;
    std::pmr::vector<struct epoll_event> events_;
};


#endif //MINIHTTP_EVENTLOOP_H
