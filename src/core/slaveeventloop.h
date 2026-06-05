//
// Created by 66 on 2026/6/5.
//

#ifndef MINIHTTP_SLAVEEVENTLOOP_H
#define MINIHTTP_SLAVEEVENTLOOP_H

#include <atomic>
#include <unordered_set>
#include <sys/timerfd.h>

#include "eventloop.h"
#include "connection.h"

class SlaveEventLoop : public EventLoop {
public:
    SlaveEventLoop();

    ~SlaveEventLoop() override;

    void addConnection(Connection *conn) { connections_.insert(conn); }

    void removeConnection(Connection *conn) { connections_.erase(conn); }

    const std::unordered_set<Connection *> &connections() const { return connections_; }

    void closeAllConnection();

    void setTimeout(int seconds) { timeout_seconds_.store(seconds, std::memory_order_relaxed); }

private:
    void handleTimeout();

    std::unordered_set<Connection *> connections_;
    Channel *timer_channel_;
    std::atomic<int> timeout_seconds_{30};
};


#endif //MINIHTTP_SLAVEEVENTLOOP_H
