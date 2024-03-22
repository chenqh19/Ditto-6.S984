#include "webserver.h"
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#include <string>
#include <iostream>

using namespace std;

WebServer::WebServer()
{
    conns_ = new TCPConn[MAX_FD];
}

WebServer::~WebServer()
{
    close(epoll_fd_);
    close(listen_fd_);
    delete[] conns_;
    delete network_thread_pool_;
}

void WebServer::Init(int port, int network_thread_number, int worker_thread_number, bool dispatch, bool running_assembly)
{
    server_port_ = port;
    network_thread_number_ = network_thread_number;
    worker_thread_number_ = worker_thread_number;
    dispatch_ = dispatch;
    network_epoll_fds_ = new int[network_thread_number_];
    running_assembly_ = running_assembly;

    mem_data_ = new uint64_t[256 * 1024 * 1024 / 8];
}

void WebServer::InitializeThreadPool()
{
    worker_thread_pool_ = new WorkerThreadPool<TCPConn>(worker_thread_number_, mem_data_);
    network_thread_pool_ = new NetworkThreadPool<TCPConn>(conns_, network_epoll_fds_, network_thread_number_, dispatch_, worker_thread_pool_, future_thread_pool_, mem_data_);
    sleep(1);
}

void WebServer::EventListen()
{
    listen_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(listen_fd_ >= 0);

    // Not use linger.
    struct linger tmp = {0, 1};
    setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(server_port_);

    int reuse_addr_flag = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_flag, sizeof(reuse_addr_flag));
    ret = bind(listen_fd_, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listen_fd_, 5);
    assert(ret >= 0);

    epoll_fd_ = epoll_create(5);
    assert(epoll_fd_ != -1);

    addfd(epoll_fd_, listen_fd_, false, 0);
}

bool WebServer::HandleNewConn(int epoll_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);

    int connfd = accept(listen_fd_, (struct sockaddr *)&client_address, &client_addrlength);
    int yes = 1;
    setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));
    if (connfd < 0)
    {
        printf("%s:errno is:%d", "accept error", errno);
        return false;
    }
    conns_[connfd].Init(epoll_fd, connfd, mem_data_, running_assembly_);

#ifdef MONGODB
    thread spawned_thread = thread(&blockingThread, connfd, &conns_[connfd], mem_data_);
    spawned_thread.detach();
#endif

    return true;
}

void WebServer::EventLoop()
{
    bool stop_server = false;
    int turn = 0;
    epoll_event events[MAX_EVENT_NUMBER];

    while (!stop_server)
    {
        int number = epoll_wait(epoll_fd_, events, MAX_EVENT_NUMBER, -1);

        if (number < 0 && errno != EINTR)
        {
            printf("%s\n", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            // Handling new connection.
            if (sockfd == listen_fd_)
            {
                // Using round-robin to dispatch connections to network threads if using pool.
                int epollfd = network_epoll_fds_[turn++];
                if (turn == network_thread_number_)
                    turn = 0;
                
                bool flag = HandleNewConn(epollfd);
                if (flag == false)
                    continue;
            }
            else
            {
                printf("%s\n", "main thread receives unknown fd");
            }
        }
    }
}