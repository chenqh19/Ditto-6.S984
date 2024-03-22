#ifndef CONFIG_H
#define CONFIG_H

#include "webserver.h"

using namespace std;

class Config
{
public:
    Config();
    ~Config(){};

    void ParseArg(int argc, char*argv[]);

    // Web server port
    int server_port;

    // Pool size of network threads.
    int network_thread_number = 4;

    // Pool size of worker threads.
    int worker_thread_number = 4;

    // Workflow model.
    bool dispatch;

    // Whether running injected assembly codes.
    bool running_assembly;
};

#endif