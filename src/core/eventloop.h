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
#include <thread>

#include "core/channel.h"
#include "net/socket.h"

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();

    virtual ~EventLoop();

    void addChannel(Channel *ch);

    void removeChannel(Channel *ch);

    void updateChannel(Channel *ch);

    void queueInLoop(Functor function);

    void runInLoop(Functor function);

    bool isInLoopThread() const;

    virtual void loop();

    void mStop();

private:
    void doPendingFunctor();

    int epfd_;
    std::vector<struct epoll_event> events_;

    Socket eventfd_;
    Channel *wakeup_channel_;
    std::mutex mutex_;
    std::vector<Functor> pending_functors_;

    std::thread::id thread_id_;
    bool quit_ = false;
};


#endif //MINIHTTP_EVENTLOOP_H
