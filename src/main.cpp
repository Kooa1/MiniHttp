#include <iostream>
#include <cstring>
#include <memory>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#include "core/eventloop.h"
#include "core/channel.h"
#include "http/parser.h"

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

    Channel accept_channel(server_fd, EPOLLIN);

    accept_channel.setReadCallBack([&loop, server_fd]() {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_len);

        std::cout << "New connection, fd=" << client_fd
                << " from " << inet_ntoa(client_addr.sin_addr)
                << ":" << ntohs(client_addr.sin_port) << std::endl;

        auto *conn_ch = new Channel(client_fd, EPOLLIN);

        conn_ch->setReadCallBack([client_fd, conn_ch, &loop]() {
            static Parser parser;
            char buf[4096];
            int n = read(client_fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                size_t consumed = parser.parse(buf, n);

                if (parser.isDone()) {
                    const Request &req = parser.getRequest();

                    std::cout << "Method: " << (req.method() == Request::GET ? "GET" : "POST") << std::endl;
                    std::cout << "URI: " << req.uri() << std::endl;
                    std::cout << "Host: " << req.header("Host") << std::endl;

                    const std::string http_response =
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 12\r\n"
                            "\r\n"
                            "Hello World!";

                    send(client_fd, http_response.data(), http_response.size(), 0);
                }
            }

            loop.removeChannel(conn_ch);
            close(client_fd);
            delete conn_ch;
        });

        loop.addChannel(conn_ch);
    });

    loop.addChannel(&accept_channel);
    loop.loop();

    close(server_fd);

    return 0;
}
