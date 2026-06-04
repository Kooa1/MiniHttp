//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_EVENTLOOP_H
#define MINIHTTP_EVENTLOOP_H

#include <mutex>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "core/channel.h"

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();

    ~EventLoop();

    void addChannel(Channel *ch);

    void removeChannel(Channel *ch);

    void updateChannel(Channel *ch);

    void queueInLoop(Functor function);

    void loop();

private:
    void doPendingFunctor();

    int epfd_;
    std::vector<struct epoll_event> events_;

    int eventfd_;
    Channel *wakeup_channel_;
    std::mutex mutex_;
    std::vector<Functor> pending_functors_;
};


#endif //MINIHTTP_EVENTLOOP_H
