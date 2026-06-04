#include <csignal>

#include "server/httpserver.h"
#include "handler/routeregister.h"

int main() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, nullptr);

    HttpServer server(8080, 4);

    RouteRegister::Register(server);

    server.mStart();

    return 0;
}
