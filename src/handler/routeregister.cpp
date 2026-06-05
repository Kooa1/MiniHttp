//
// Created by 66 on 2026/6/4.
//

#include "routeregister.h"

void RouteRegister::Register(HttpServer &server) {
    server.use([](const Request &req, Connection *conn, Middleware::Next next) {
        std::cout << "[" << req.methodStr() << "] " << req.uri() << std::endl;
        next();
    });

    server.Get("/", [](const Request &req, Connection *conn) {
        conn->mSend("Hello World!");
    });

    server.Get("/about", [](const Request &req, Connection *conn) {
        conn->mSend("This is my server");
    });

    // --- 动态路由：:id 参数 ---
    server.Get("/user/:id", [](const Request &req, Connection *conn) {
        conn->mSend("User: " + req.param("id"));
    });

    // --- 多段动态路由 ---
    server.Get("/user/:id/post/:pid", [](const Request &req, Connection *conn) {
        conn->mSend("User: " + req.param("id") + ", Post: " + req.param("pid"));
    });

    // --- 线程池示例：CPU 密集计算在 worker 线程执行，结果交回 IO 线程发送 ---
    server.Get("/slow", [&server](const Request &req, Connection *conn) {
        // 拷贝 Request：parser_ 归 Connection 所有，引用会悬空
        Request req_copy = req;
        auto alive = conn->aliveToken();
        // 提交到线程池，不阻塞 IO 线程
        server.threadPool().submit([&server, conn, req_copy, alive]() {
            // 工作线程：模拟 CPU 密集计算
            std::string result = "Heavy computation for " + req_copy.uri();

            conn->loop()->queueInLoop([conn, result, alive]() {
                if (!*alive) return;
                conn->mSend(result);
                conn->cleanupAfterSend();
            });
        });
    });

    server.Get("/api/routes", [](const Request &req, Connection *conn) {
        std::ostringstream json;
        json << R"({"routes":[)" "\n"
             << R"(  {"method":"GET","path":"/","handler":"Hello World!"},)" "\n"
             << R"(  {"method":"GET","path":"/about","handler":"This is my server"},)" "\n"
             << R"(  {"method":"GET","path":"/user/:id","handler":"User: {id}"},)" "\n"
             << R"(  {"method":"GET","path":"/user/:id/post/:pid","handler":"User: {id}, Post: {pid}"},)" "\n"
             << R"(  {"method":"GET","path":"/slow","handler":"Heavy computation demo"},)" "\n"
             << R"(  {"method":"GET","path":"/api/routes","handler":"This listing"})" "\n"
             << R"(]})";
        conn->mSend(json.str());
    });

    server.ServerStatic("/static", "/home/shanhai/tmp/tmp.Tj7Vov2HvY/MiniHttp/www");
}
