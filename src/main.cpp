#include "server/httpserver.h"

int main() {
    HttpServer server(8080, 4);

    server.Get("/", [](const Request &req, Connection *conn) {
        conn->mSend("Hello World!");
        conn->mClose();
    });
    server.Get("/about", [](const Request &req, Connection *conn) {
        conn->mSend("This is my server");
        conn->mClose();
    });
    server.Get("/user/:id", [](const Request &req, Connection *conn) {
        conn->mSend("User: " + req.param("id"));
        conn->mClose();
    });
    server.Get("/user/:id/post/:pid", [](const Request &req, Connection *conn) {
        conn->mSend("User: " + req.param("id") + ", Post: " + req.param("pid"));
        conn->mClose();
    });

    server.Get("/slow", [&server](const Request &req, Connection *conn) {
        Request req_copy = req;

        server.threadPool().submit([&server, conn, req_copy]() {
            std::string result = "Heavy computation for " + req_copy.uri();

            server.loop().queueInLoop([conn, result]() {
                conn->mSend(result);
                conn->mClose();
            });
        });
    });

    server.mStart();

    return 0;
}
