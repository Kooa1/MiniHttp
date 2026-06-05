//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_CONNECTION_H
#define MINIHTTP_CONNECTION_H

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <fcntl.h>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <map>

#include "channel.h"
#include "eventloop.h"
#include "http/parser.h"
#include "net/socket.h"
#include "net/buffer.h"

class Connection {
public:
    using CloseCallback = std::function<void()>;
    using RequestCallback = std::function<void(const Request &req, Connection *conn)>;

    Connection(EventLoop *loop, Socket fd);

    ~Connection();

    int fd() const { return fd_.get(); }

    void handleRead();

    void mClose();

    void setCallback(CloseCallback cb) { close_callback_ = std::move(cb); }

    void setRequestCallback(RequestCallback cb);

    void mSend(const std::string &body,
               int status_code = 200,
               const std::string &content_type = "text/plain");

    void cleanupAfterSend();

    EventLoop *loop() const { return loop_; }

    std::shared_ptr<bool> aliveToken() const { return alive_; }

    void updateActiveTime() { last_active_ = std::chrono::steady_clock::now(); }

    std::chrono::steady_clock::time_point lastActive() const { return last_active_; }

    std::map<std::string, std::string> attributes_;

    void setSent(bool s) { sent_ = s; }

    void setKeepAlive(bool ka) { keep_alive_ = ka; }

private:
    static constexpr size_t kMaxBufferSize = 65536;

    EventLoop *loop_;
    Socket fd_;
    Channel channel_;
    Parser parser_;
    Buffer input_buffer_;
    CloseCallback close_callback_;
    RequestCallback request_callback_;
    bool close_ = false;
    bool sent_ = false;
    bool keep_alive_ = false;

    static const std::unordered_map<int, std::string> kStatusText;
    std::shared_ptr<bool> alive_;

    std::chrono::steady_clock::time_point last_active_;
};


#endif //MINIHTTP_CONNECTION_H
