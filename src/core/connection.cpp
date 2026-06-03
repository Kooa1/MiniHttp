//
// Created by 66 on 2026/6/3.
//

#include "connection.h"

#include <sys/socket.h>

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
        if (parser_.isDone()) {
            std::cout << "Parsed request: " << parser_.getRequest().methodStr()
                    << " " << parser_.getRequest().uri() << std::endl;

            const std::string http_response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 12\r\n"
                    "\r\n"
                    "Hello World!";

            send(fd_, http_response.data(), http_response.size(), 0);
            closeFD();
        }
    } else {
        closeFD();
    }
}

void Connection::closeFD() {
    loop_->removeChannel(&channel_);
    close(fd_);
    if (close_callback_) close_callback_();
    delete this;
}
