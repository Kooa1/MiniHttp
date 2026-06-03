//
// Created by 66 on 2026/6/3.
//

#include "router.h"

void Router::Get(const std::string &path, Handler handler) {
    router_["GET"][path] = std::move(handler);
}

void Router::Post(const std::string &path, Handler handler) {
    router_["POST"][path] = std::move(handler);
}

void Router::dispatch(const Request &req, Connection *conn) {
    auto method_it = router_.find(req.methodStr());
    if (method_it == router_.end()) {
        conn->mSend("Method Not Allowed", 405);
        return;
    }

    auto &path_map = method_it->second;
    auto path_it = path_map.find(req.uri());
    if (path_it != path_map.end()) {
        path_it->second(req, conn);
    } else {
        conn->mSend("Not Found", 404);
    }
}
