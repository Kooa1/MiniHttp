//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_CONNECTION_H
#define MINIHTTP_CONNECTION_H

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <fcntl.h>

#include "channel.h"
#include "eventloop.h"
#include "http/parser.h"

class Connection {
public:
    using CloseCallback = std::function<void()>;
    using RequestCallback = std::function<void(const Request &req, Connection *conn)>;

    Connection(EventLoop *loop, int fd);

    ~Connection();

    int fd() const { return fd_; };

    void handleRead();

    void mClose();

    void setCallback(CloseCallback cb) { close_callback_ = std::move(cb); };

    void setRequestCallback(RequestCallback cb);

    void mSend(const std::string &body,
               int status_code = 200,
               const std::string &content_type = "text/plain");

private:
    EventLoop *loop_;
    int fd_;
    Channel channel_;
    Parser parser_;
    std::string input_buffer_;
    CloseCallback close_callback_;
    RequestCallback request_callback_;
    bool close_ = false;
    bool sent_ = false;

    static const std::unordered_map<int, std::string> kStatusText;
};


#endif //MINIHTTP_CONNECTION_H
