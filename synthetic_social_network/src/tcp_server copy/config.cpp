#include "config.h"

Config::Config(){
    // Web server port, default 9016.
    server_port = 9000;

    // Pool size of network threads, default 4
    network_thread_number = 1;

    // Pool size of worker threads, default 4
    worker_thread_number = 1;

    // Workflow model, default inline.
    dispatch = false;

    // Whether running injected assembly codes, default true.
    running_assembly = true;
}

void Config::ParseArg(int argc, char*argv[]){
    int opt;
    const char *str = "p:n:w:a:d:e:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            server_port = atoi(optarg);
            break;
        }
        case 'n':
        {
            network_thread_number = atoi(optarg);
            break;
        }
        case 'w':
        {
            worker_thread_number = atoi(optarg);
            break;
        }
        case 'a':
        {
            if (atoi(optarg) == 0) {
                running_assembly =  false;
            }
            break;
        }
        case 'd':
        {
            if (atoi(optarg) == 1) {
                dispatch =  true;
            }
            break;
        }
        default:
            break;
        }
    }
}