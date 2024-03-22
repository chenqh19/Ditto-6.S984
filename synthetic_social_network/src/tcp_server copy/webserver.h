#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "./connection/tcp_conn.h"
#include "./threadpool/threadpool.h"
#include "./utils.h"

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void Init(int port, int network_thread_number, int worker_thread_number, bool dispatch, bool running_assembly);
    void InitializeThreadPool();
    void EventListen();
    void EventLoop();

private:
    bool HandleNewConn(int epoll_fd);

public:
    int server_port_;
    int epoll_fd_;
    bool dispatch_;
    TCPConn* conns_;

    NetworkThreadPool<TCPConn>* network_thread_pool_;
    WorkerThreadPool<TCPConn>* worker_thread_pool_;
    FutureThreadPool* future_thread_pool_;
    int network_thread_number_;
    int worker_thread_number_;
    int* network_epoll_fds_;    
    int listen_fd_;

    uint64_t* mem_data_;
    bool running_assembly_;
};

#endif