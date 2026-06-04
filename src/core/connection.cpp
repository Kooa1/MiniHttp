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
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
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

            if (!close_ && sent_) {
                std::cout << "DEBUG handleRead: keep_alive_=" << keep_alive_ << std::endl;
                if (keep_alive_) {
                    parser_.reset();
                    input_buffer_.clear();
                    sent_ = false;
                    keep_alive_ = false;
                } else {
                    std::cout << "read<=0, client closed fd=" << fd_ << std::endl;
                    mClose();
                }
            }
        }
    } else {
        std::cout << "keep-alive=false, closing fd=" << fd_ << std::endl;
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

    if (parser_.isDone()) {
        const auto &req = parser_.getRequest();
        std::string conn = req.header("connction");
        if (conn == "keep-alive") {
            keep_alive_ = true;
        } else if (conn == "close") {
            keep_alive_ = false;
        } else if (req.version() == Request::HTTP_1_1) {
            keep_alive_ = true;
        }
    }

    auto it = kStatusText.find(status_code);
    std::string reason = (it != kStatusText.end()) ? it->second : "Unknown";

    std::ostringstream response;
    response << "HTTP/1.1 " << std::to_string(status_code) << " " << reason << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << std::to_string(body.size()) << "\r\n";

    if (keep_alive_) {
        response << "Connection: keep-alive\r\n";
    }

    response << "\r\n";
    response << body;

    send(fd_, response.str().data(), response.str().size(), 0);
}
