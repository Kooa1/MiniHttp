#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#include "core/eventloop.h"
#include "core/channel.h"
#include "core/connection.h"
#include "http/router.h"

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
    listen(server_fd, 128);

    std::cout << "Server listening on http://localhost:8080" << std::endl;

    EventLoop loop;
    Router router;
    router.Get("/", [](const Request &req, Connection *conn) {
        conn->mSend("Hello World!");
    });
    router.Get("/about", [](const Request &req, Connection *conn) {
        conn->mSend("This is my server");
    });

    Channel accept_channel(server_fd, EPOLLIN);

    accept_channel.setReadCallBack([&loop, server_fd, &router]() {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_len);

        std::cout << "New connection, fd=" << client_fd
                << " from " << inet_ntoa(client_addr.sin_addr)
                << ":" << ntohs(client_addr.sin_port) << std::endl;

        auto *conn = new Connection(&loop, client_fd);
        conn->setRequestCallback([&router](const Request &req, Connection *conn) {
            router.dispatch(req, conn);
            conn->mClose();
        });
    });

    loop.addChannel(&accept_channel);
    loop.loop();

    close(server_fd);

    return 0;
}
