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
        return;
    }

    std::vector<std::string> req_segments = StringUtil::SplitPath(req.uri());

    for (const auto &[pattern, handler]: path_map) {
        std::vector<std::string> pattern_segments = StringUtil::SplitPath(pattern);

        if (req_segments.size() != pattern_segments.size()) continue;

        Request params_req = req;
        bool match = true;

        for (size_t i = 0; i < pattern_segments.size(); i++) {
            if (pattern_segments[i][0] == ':') {
                std::string param_name = pattern_segments[i].substr(1);
                params_req.setParam(param_name, req_segments[i]);
            } else if (pattern_segments[i] != req_segments[i]) {
                match = false;
                break;
            }
        }

        if (match) {
            handler(params_req, conn);
            return;
        }
    }

    conn->mSend("Not Found", 404);
}
