#include "server/httpserver.h"
#include "handler/routeregister.h"

int main() {
    HttpServer server(8080, 4);

    RouteRegister::Register(server);

    server.mStart();

    return 0;
}
