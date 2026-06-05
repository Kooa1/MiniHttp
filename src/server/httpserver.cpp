//
// Created by 66 on 2026/6/4.
//

#include "httpserver.h"

namespace {
    int createSignalFD() {
        sigset_t mask; // 与 main.cpp 用同一组信号
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGTERM);
        return signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    }
}

HttpServer::HttpServer(const uint64_t port, size_t io_thread, const size_t thread_count)
    : event_loop_group_(io_thread),
      thread_pool_(thread_count),
      server_fd_(Socket(::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0))),
      accept_channel_(server_fd_.get(), EPOLLIN),
      signal_fd_(createSignalFD()),
      signal_channel_(signal_fd_.get(), EPOLLIN) {
    constexpr int opt = 1;
    setsockopt(server_fd_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd_.get(), reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
    listen(server_fd_.get(), 128);

    std::cout << "Server listening on http://localhost:" << port << std::endl;

    accept_channel_.setReadCallBack([this]() {
        onAccept();
    });

    signal_channel_.setReadCallBack([this]() {
        struct signalfd_siginfo info;
        read(signal_fd_.get(), &info, sizeof(info));
        std::cout << "Shutting down (signal " << info.ssi_signo << ")..." << std::endl;
        loop_.mStop();
    });
}

HttpServer::~HttpServer() {
    mStop();
}

void HttpServer::Get(const std::string &path, Router::Handler handler) {
    router_.Get(path, std::move(handler));
}

void HttpServer::Post(const std::string &path, Router::Handler handler) {
    router_.Post(path, std::move(handler));
}

EventLoop &HttpServer::loop() {
    return loop_;
}

ThreadPool &HttpServer::threadPool() {
    return thread_pool_;
}

void HttpServer::mStart() {
    loop_.addChannel(&signal_channel_);
    loop_.addChannel(&accept_channel_);

    event_loop_group_.mStart();

    loop_.loop();

    mStop();
}

void HttpServer::setTimeout(int seconds) {
    event_loop_group_.setTimeout(seconds);
}

void HttpServer::use(Middleware::Func mw) {
    middleware_.use(std::move(mw));
}

void HttpServer::onRequest(const Request &req, Connection *conn) {
    try {
        middleware_.run(req, conn, [this, &req, conn]() {
            router_.dispatch(req, conn);
        });
    } catch (const std::exception &e) {
        conn->mSend("Internal Server Error", 500);
        conn->cleanupAfterSend();
    }
}

void HttpServer::onAccept() {
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    Socket client_fd(
        accept(server_fd_.get(),
               reinterpret_cast<struct sockaddr *>(&client_addr),
               &client_len));

    std::cout << "New connection, fd=" << client_fd.get()
            << " from " << inet_ntoa(client_addr.sin_addr)
            << ":" << ntohs(client_addr.sin_port) << std::endl;

    auto *slave = event_loop_group_.next();
    int raw_fd = client_fd.release();
    slave->runInLoop([slave, this, raw_fd]() mutable {
        auto *conn = new Connection(slave, Socket(raw_fd));

        conn->setCallback([slave, conn]() {
            std::cout << "Connection " << conn->fd() << " removed" << std::endl;
            slave->removeConnection(conn);
        });

        conn->setRequestCallback([this](const Request &req, Connection *conn) {
            onRequest(req, conn);
        });

        slave->addConnection(conn);
    });
}

void HttpServer::mStop() {
    if (stopped_) return;
    stopped_ = true;

    loop_.removeChannel(&accept_channel_);
    loop_.removeChannel(&signal_channel_);

    event_loop_group_.stopAll();

    threadPool().mStop();
}
