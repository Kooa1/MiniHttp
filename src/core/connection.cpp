//
// Created by 66 on 2026/6/3.
//

#include "connection.h"

const std::unordered_map<int, std::string> Connection::kStatusText = {
    {200, "ok"},
    {404, "Not Found"},
    {500, "Internal Server Error"}
};

Connection::Connection(EventLoop *loop, int fd) : loop_(loop), fd_(fd), channel_(fd, EPOLLIN) {
    channel_.setReadCallBack([this]() {
        handleRead();
    });

    loop->addChannel(&channel_);
}

Connection::~Connection() {
}

void Connection::handleRead() {
    char buf[4096];
    int n = read(fd_, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        input_buffer_ += buf;

        size_t consumed = parser_.parse(input_buffer_.data(), input_buffer_.size());
        input_buffer_.erase(0, consumed);
        if (parser_.isDone()) {
            if (request_callback_) {
                request_callback_(parser_.getRequest(), this);
            }
        }
    } else {
        mClose();
    }
}

void Connection::mClose() {
    if (close_) return;
    close_ = true;

    loop_->removeChannel(&channel_);
    close(fd_);
    if (close_callback_) close_callback_();
    delete this;
}

void Connection::setRequestCallback(RequestCallback cb) {
    request_callback_ = std::move(cb);
}

void Connection::mSend(const std::string &body, int status_code, const std::string &content_type) {
    if (sent_) return;
    sent_ = true;

    auto it = kStatusText.find(status_code);
    std::string reason = (it != kStatusText.end()) ? it->second : "Unknown";

    std::ostringstream response;
    response << "HTTP/1.1 " << std::to_string(status_code) << " " << reason << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << std::to_string(body.size()) << "\r\n";
    response << "\r\n";
    response << body;

    send(fd_, response.str().data(), response.str().size(), 0);
}
