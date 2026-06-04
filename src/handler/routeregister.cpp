//
// Created by 66 on 2026/6/4.
//

#include "routeregister.h"

void RouteRegister::Register(HttpServer &server) {
    server.Get("/", [](const Request &req, Connection *conn) {
        conn->mSend("Hello World!");
        conn->mClose();
    });

    server.Get("/about", [](const Request &req, Connection *conn) {
        conn->mSend("This is my server");
        conn->mClose();
    });

    // --- 动态路由：:id 参数 ---
    server.Get("/user/:id", [](const Request &req, Connection *conn) {
        conn->mSend("User: " + req.param("id"));
        conn->mClose();
    });

    // --- 多段动态路由 ---
    server.Get("/user/:id/post/:pid", [](const Request &req, Connection *conn) {
        conn->mSend("User: " + req.param("id") + ", Post: " + req.param("pid"));
        conn->mClose();
    });

    // --- 线程池示例：CPU 密集计算在 worker 线程执行，结果交回 IO 线程发送 ---
    server.Get("/slow", [&server](const Request &req, Connection *conn) {
        // 拷贝 Request：parser_ 归 Connection 所有，引用会悬空
        Request req_copy = req;

        // 提交到线程池，不阻塞 IO 线程
        server.threadPool().submit([&server, conn, req_copy]() {
            // 工作线程：模拟 CPU 密集计算
            std::string result = "Heavy computation for " + req_copy.uri();

            // 交回 IO 线程发送响应
            server.loop().queueInLoop([conn, result]() {
                conn->mSend(result);
                conn->mClose();
            });
        });
    });
}
