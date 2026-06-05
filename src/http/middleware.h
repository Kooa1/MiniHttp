//
// Created by 66 on 2026/6/5.
//

#ifndef MINIHTTP_MIDDLEWARE_H
#define MINIHTTP_MIDDLEWARE_H

#include <functional>

#include "core/connection.h"
#include "http/request.h"

class Middleware {
public:
    using Next = std::function<void()>;

    using Func = std::function<void(const Request &req, Connection *conn, Next next)>;

    void use(Func mw);

    void run(const Request &req, Connection *conn, std::function<void()> final);

private:
    std::vector<Func> chain_;
};


#endif //MINIHTTP_MIDDLEWARE_H
