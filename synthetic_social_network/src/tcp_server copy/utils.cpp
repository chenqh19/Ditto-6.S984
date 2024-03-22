#include "utils.h"

#include <chrono>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "connection/tcp_conn.h"
#include "threadpool/threadpool.h"

using std::chrono::microseconds;
using std::chrono::duration_cast;
using std::chrono::system_clock;

//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//从内核时间表删除描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void test_future(int time) {
    auto start = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    while (true) {
        auto current = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        if (current - start >= (int)(time))
            break;
    }
}

void test_nested(FutureThreadPool* future_pool, int time) {
    auto task_future = future_pool->Enqueue(&test_future, time);
    task_future.wait();
}

void* test_pthread(void *arg) {
    int time = 50;
    auto start = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    while (true) {
        auto current = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        if (current - start >= (int)(time))
            break;
    }
}

void* blockingThread(int connfd, TCPConn* conn, uint64_t* mem_data)
{
    uint64_t* curr_mem_addrs = new uint64_t[23];
    uint64_t cache_init_size = 64;
    uint64_t cache_max_size = 256 * 1024 * 1024;

    uint64_t curr_idx = 0;
    for (int i = 0; i < 23; i++) {
        curr_mem_addrs[i] = (uint64_t)&mem_data[curr_idx];
        curr_idx = (cache_init_size << i) / 8;
    }
    uint64_t request_id = 0;

    for(;;) {
        conn->ReadOnce();
        conn->Process(curr_mem_addrs, request_id);
        conn->WriteOnce();
    }
    close(connfd);
}