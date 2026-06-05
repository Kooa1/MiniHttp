//
// Created by 66 on 2026/6/4.
//

#ifndef MINIHTTP_ROUTEREGISTER_H
#define MINIHTTP_ROUTEREGISTER_H

#include "server/httpserver.h"
#include "core/connection.h"
#include "http/middleware.h"

class RouteRegister {
public:
    static void Register(HttpServer &server);
};


#endif //MINIHTTP_ROUTEREGISTER_H
