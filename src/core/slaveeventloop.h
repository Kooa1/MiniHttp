//
// Created by 66 on 2026/6/5.
//

#ifndef MINIHTTP_SLAVEEVENTLOOP_H
#define MINIHTTP_SLAVEEVENTLOOP_H

#include <unordered_set>

#include "eventloop.h"
#include "connection.h"

class SlaveEventLoop : public EventLoop {
public:
    SlaveEventLoop() = default;

    ~SlaveEventLoop() override = default;

    void addConnection(Connection *conn) { connections_.insert(conn); }

    void removeConnection(Connection *conn) { connections_.erase(conn); }

    const std::unordered_set<Connection *> &connections() const { return connections_; }

    void closeAllConnection();

private:
    std::unordered_set<Connection *> connections_;
};


#endif //MINIHTTP_SLAVEEVENTLOOP_H
