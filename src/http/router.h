//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_ROUTER_H
#define MINIHTTP_ROUTER_H

#include "request.h"
#include "core/connection.h"

class Router {
public:
    using Handler = std::function<void(const Request &req, Connection *conn)>;

    void Get(const std::string &path, Handler handler);

    void Post(const std::string &path, Handler handler);

    void dispatch(const Request &req, Connection *conn);

private:
    std::unordered_map<std::string, std::map<std::string, Handler> > router_;
};


#endif //MINIHTTP_ROUTER_H
