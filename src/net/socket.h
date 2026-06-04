//
// Created by 66 on 2026/6/4.
//

#ifndef MINIHTTP_SOCKET_H
#define MINIHTTP_SOCKET_H

#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>

class Socket {
public:
    explicit Socket();

    explicit Socket(int fd);

    ~Socket();

    Socket(Socket &&other) noexcept;

    Socket &operator=(Socket &&other) noexcept;

    Socket(const Socket &) = delete;

    Socket &operator=(const Socket &) = delete;

    int get() const { return fd_; }

    int release();

private:
    int fd_;
};


#endif //MINIHTTP_SOCKET_H
