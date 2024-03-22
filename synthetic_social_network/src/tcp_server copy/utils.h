#ifndef UTILS_H
#define UTILS_H

#include <cstdint>

class FutureThreadPool;
class TCPConn;

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

//对文件描述符设置非阻塞
int setnonblocking(int fd);

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

//从内核时间表删除描述符
void removefd(int epollfd, int fd);

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode);

void test_future(int time);

void test_nested(FutureThreadPool* future_pool, int time);

void* test_pthread(void *arg);

void* blockingThread(int connfd, TCPConn* conn, uint64_t* mem_data);

void runAssembly0(uint64_t* mem_data, uint64_t req_id, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs);

#endif