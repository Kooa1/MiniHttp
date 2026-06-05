//
// Created by 66 on 2026/6/3.
//

#include "connection.h"

const std::unordered_map<int, std::string> Connection::kStatusText = {
    {200, "OK"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"}
};

Connection::Connection(EventLoop *loop, Socket fd)
    : loop_(loop),
      fd_(std::move(fd)),
      channel_(fd_.get(), EPOLLIN),
      alive_(std::make_shared<bool>(true)),
      last_active_(std::chrono::steady_clock::now()) {
    int flags = fcntl(fd_.get(), F_GETFL, 0);
    fcntl(fd_.get(), F_SETFL, flags | O_NONBLOCK);
    channel_.setReadCallBack([this]() {
        handleRead();
    });

    loop->addChannel(&channel_);
}

Connection::~Connection() {
}

void Connection::handleRead() {
    int n = input_buffer_.readFd(fd_.get());
    if (n > 0) {
        updateActiveTime();
        size_t consumed = parser_.parse(input_buffer_.peek(), input_buffer_.readableBytes());
        if (parser_.hasError()) {
            mClose();
            return;
        }
        input_buffer_.retrieve(consumed);
        if (parser_.isDone()) {
            if (request_callback_) {
                request_callback_(parser_.getRequest(), this);
            }

            if (!close_ && sent_) {
                std::cout << "DEBUG handleRead: keep_alive_=" << keep_alive_ << std::endl;
                if (keep_alive_) {
                    parser_.reset();
                    input_buffer_.retrieveAll();
                    sent_ = false;
                    keep_alive_ = false;
                } else {
                    std::cout << "read<=0, client closed fd=" << fd_.get() << std::endl;
                    mClose();
                }
            }
        }
    } else {
        std::cout << "keep-alive=false, closing fd=" << fd_.get() << std::endl;
        mClose();
    }
}

void Connection::mClose() {
    if (close_) return;
    close_ = true;
    *alive_ = false;
    loop_->removeChannel(&channel_);
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
        std::string conn = req.header("connection");
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

    send(fd_.get(), response.str().data(), response.str().size(), 0);
}

void Connection::cleanupAfterSend() {
    if (keep_alive_ && sent_) {
        parser_.reset();
        input_buffer_.retrieveAll();
        sent_ = false;
        keep_alive_ = false;
    } else if (sent_ && !keep_alive_) {
        mClose();
    }
}
