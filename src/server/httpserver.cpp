//
// Created by 66 on 2026/6/4.
//

#include "httpserver.h"

HttpServer::HttpServer(const uint64_t port, size_t thread_count)
    : thread_pool_(thread_count),
      server_fd_(Socket()),
      accept_channel_(server_fd_.get(), EPOLLIN) {
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
}

HttpServer::~HttpServer() {
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
    loop_.addChannel(&accept_channel_);
    loop_.loop();
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

    auto *channel_accept = new Connection(&loop_, client_fd.release());
    channel_accept->setRequestCallback([this](const Request &req, Connection *conn) {
        router_.dispatch(req, conn);
    });
}
