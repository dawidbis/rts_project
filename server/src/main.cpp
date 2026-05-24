// main.cpp - rts_server entry point (skeleton)

#include <spdlog/spdlog.h>

int main(int /*argc*/, char* /*argv*/[]) {
    spdlog::info("rts-server starting (skeleton build)");
    // TODO: load configuration
    // TODO: initialize database connection (libpqxx)
    // TODO: start TCP session manager (Boost.Asio)
    // TODO: start UDP game loop (ENet)
    // TODO: expose Prometheus metrics endpoint
    spdlog::info("rts-server shutting down");
    return 0;
}
