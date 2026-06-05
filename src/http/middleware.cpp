//
// Created by 66 on 2026/6/5.
//

#include "middleware.h"

void Middleware::use(Func mw) {
    chain_.push_back(std::move(mw));
}

void Middleware::run(const Request &req, Connection *conn, std::function<void()> final) {
    if (chain_.empty()) {
        final();
        return;
    }

    size_t idx = 0;
    std::function<void()> go;
    go = [&]() {
        if (idx < chain_.size()) {
            auto &fn = chain_[idx];
            idx++;
            fn(req, conn, go);
        } else {
            final();
        }
    };
    go();
}
