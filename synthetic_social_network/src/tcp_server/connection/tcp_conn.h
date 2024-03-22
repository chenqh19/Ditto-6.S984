#ifndef TCPCONN_H
#define TCPCONN_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

class TCPConn
{
public:
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 2048;

public:
    TCPConn() {}
    ~TCPConn() {}

public:
    void Init(int epoll_fd, int socket_fd, bool running_assembly);
    void CloseConn(bool real_close = true);
    void Process(uint64_t* mem_data, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs, uint64_t &request_id);
    void ReadOnce();
    void WriteOnce();
    int ParseRequest();

private:
    void Reset();
    bool AddResponse(const char *format, ...);
    bool AddContent(const char *content);
    bool AddStatusLine(int status, const char *title);
    bool AddHeaders(int content_length);
    bool AddContentType();
    bool AddContentLength(int content_length);
    bool AddLinger();
    bool AddBlankLine();

public:
    int epoll_fd_;

private:
    int socket_fd_;
    char read_buf_[READ_BUFFER_SIZE];
    int read_idx_;
    char write_buf_[WRITE_BUFFER_SIZE];
    int write_idx_;
    struct iovec iv_[2];
    int iv_count_;
    int bytes_to_send_;
    int bytes_have_send_;


    bool running_assembly_;
    bool linger_;

#ifdef NGINX
    int nginx_write_fd_;
    char nginx_write_buf_[WRITE_BUFFER_SIZE];
#endif

#ifdef MONGODB
    int mongod_write_fd_;
    int mongod_read_fd_;
#endif

#ifdef REDIS
    int redis_read_fd_;
    char redis_read_buf_[READ_BUFFER_SIZE];
#endif

};

#endif