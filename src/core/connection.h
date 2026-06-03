//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_CONNECTION_H
#define MINIHTTP_CONNECTION_H

#include <iostream>

#include "channel.h"
#include "eventloop.h"
#include "http/parser.h"

class Connection {
public:
    using CloseCallback = std::function<void()>;

    Connection(EventLoop *loop, int fd);

    ~Connection();

    int fd() const { return fd_; };

    void handleRead();

    void closeFD();

    void setCallback(CloseCallback cb) { close_callback_ = std::move(cb); };

private:
    EventLoop *loop_;
    int fd_;
    Channel channel_;
    Parser parser_;
    std::string input_buffer_;
    CloseCallback close_callback_;
};


#endif //MINIHTTP_CONNECTION_H
