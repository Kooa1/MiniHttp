//
// Created by 66 on 2026/6/4.
//

#include "socket.h"

Socket::Socket()
    : fd_(-1) {
}

Socket::Socket(const int fd) : fd_(fd) {
    if (fd_ < 0) {
        throw std::runtime_error("socket creation failed\n");
    }
}

Socket::~Socket() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

Socket::Socket(Socket &&other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

Socket &Socket::operator=(Socket &&other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) close(fd_);
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

int Socket::release() {
    const int fd = fd_;
    fd_ = -1;
    return fd;
}
