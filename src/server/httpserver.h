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
#include <unordered_set>
#include <signal.h>
#include <sys/signalfd.h>

#include "core/eventloop.h"
#include "core/eventloopgroup.h"
#include "core/slaveeventloop.h"
#include "core/connection.h"
#include "http/router.h"
#include "http/middleware.h"
#include "thread/threadpool.h"
#include "../net/socket.h"


class HttpServer {
public:
    explicit HttpServer(const uint64_t port,
                        size_t io_thread = 2,
                        size_t thread_count = 4);

    HttpServer(const HttpServer &) = delete;

    HttpServer &operator=(const HttpServer &) = delete;

    ~HttpServer();

    void Get(const std::string &path, Router::Handler handler);

    void Post(const std::string &path, Router::Handler handler);

    EventLoop &loop();

    ThreadPool &threadPool();

    void mStart();

    void setTimeout(int seconds);

    void use(Middleware::Func mw);

    void onRequest(const Request &req, Connection *conn);

private:
    void onAccept();

    void mStop();

    EventLoop loop_;
    EventLoopGroup event_loop_group_;
    ThreadPool thread_pool_;
    Middleware middleware_;
    Router router_;

    Socket server_fd_;
    Channel accept_channel_;

    Socket signal_fd_;
    Channel signal_channel_;

    bool stopped_ = false;
};


#endif //MINIHTTP_HTTPSERVER_H
