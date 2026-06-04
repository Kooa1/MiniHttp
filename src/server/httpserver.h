//
// Created by 66 on 2026/6/4.
//

#ifndef MINIHTTP_HTTPSERVER_H
#define MINIHTTP_HTTPSERVER_H

#include <string>
#include <cstdint>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "core/eventloop.h"
#include "core/connection.h"
#include "http/router.h"
#include "thread/threadpool.h"
#include "resources/socket.h"


class HttpServer {
public:
    explicit HttpServer(const uint64_t port, size_t thread_count = 4);

    HttpServer(const HttpServer &) = delete;

    HttpServer &operator=(const HttpServer &) = delete;

    ~HttpServer();

    void Get(const std::string &path, Router::Handler handler);

    void Post(const std::string &path, Router::Handler handler);

    EventLoop &loop();

    ThreadPool &threadPool();

    void mStart();

private:
    void onAccept();

    EventLoop loop_;
    ThreadPool thread_pool_;
    Router router_;

    Socket server_fd_;
    Channel accept_channel_;
};


#endif //MINIHTTP_HTTPSERVER_H
