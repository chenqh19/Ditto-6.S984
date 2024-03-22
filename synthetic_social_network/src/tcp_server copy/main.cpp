#include "../tcp_server/config.h"

int main(int argc, char *argv[])
{
    Config config;
    config.ParseArg(argc, argv);

    WebServer server;
    server.Init(config.server_port, config.network_thread_number, config.worker_thread_number, config.dispatch, config.running_assembly);

    server.InitializeThreadPool();
    server.EventListen();
    server.EventLoop();

    return 0;
}