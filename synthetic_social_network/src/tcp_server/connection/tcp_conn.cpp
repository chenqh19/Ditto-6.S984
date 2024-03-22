#include "tcp_conn.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <cstring>
#include <string>
#include <iostream>

#include "../utils.h"

using namespace std;

const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";

// Closes a connection.
void TCPConn::CloseConn(bool real_close)
{
    if (real_close && (socket_fd_ != -1))
    {
#ifdef MEMCACHED
        removefd(epoll_fd_, socket_fd_);
#endif

#ifdef NGINX
        removefd(epoll_fd_, socket_fd_);
#endif

#ifdef REDIS
        removefd(epoll_fd_, socket_fd_);
#endif

        socket_fd_ = -1;
    }
}

// Resetializes a connection and registers the sockfd to the epollfd monitored by the network thread assigned to this connection.
void TCPConn::Init(int epoll_fd, int socket_fd,  bool running_assembly)
{
    epoll_fd_ = epoll_fd;
    socket_fd_ = socket_fd;
    // mem_data_ = mem_data;
    // pointer_chasing_mem_data_ = pointer_chasing_mem_data;
    running_assembly_ = running_assembly;

#ifdef MEMCACHED
    addfd(epoll_fd_, socket_fd_, false, 0);
#endif

#ifdef NGINX
    nginx_write_fd_ = open("nginx.log", O_CREAT | O_WRONLY | O_NONBLOCK | O_APPEND);
    addfd(epoll_fd_, socket_fd_, false, 0);
#endif

#ifdef MONGODB
    mongod_write_fd_ = open("mongod.log", O_CREAT | O_WRONLY | O_NONBLOCK | O_APPEND);
    mongod_read_fd_ = open("index.html", O_RDONLY | O_NONBLOCK);
    if (mongod_read_fd_ == -1) {
        printf("%s\n", "MongoDB cannot open read file.");
    }
#endif

#ifdef REDIS
    redis_read_fd_ = open("index.html", O_RDONLY | O_NONBLOCK);
    if (redis_read_fd_ == -1) {
        printf("%s\n", "Redis cannot open read file.");
    }
    pread(redis_read_fd_, redis_read_buf_, 2000, 0);
    addfd(epoll_fd_, socket_fd_, false, 0);
#endif

    Reset();
}

// Resets for handling new requests.
void TCPConn::Reset()
{
    bytes_to_send_ = 0;
    bytes_have_send_ = 0;
    read_idx_ = 0;
    write_idx_ = 0;
    // Sets true for connection keep-alive.
    linger_ = true;

    memset(read_buf_, '\0', READ_BUFFER_SIZE);
    memset(write_buf_, '\0', WRITE_BUFFER_SIZE);
}

// Reads data from socket until no available data or the socket is closed.
// Specially, for ET mode, it needs to read all data at once.
void TCPConn::ReadOnce()
{
    int bytes_read = 0;

#ifdef MEMCACHED
    bytes_read = read(socket_fd_, read_buf_ + read_idx_, READ_BUFFER_SIZE - read_idx_);
#endif

#ifdef NGINX
    bytes_read = recv(socket_fd_, read_buf_ + read_idx_, READ_BUFFER_SIZE - read_idx_, 0);
#endif

#ifdef MONGODB
    struct msghdr msg;
    struct iovec iov;
    iov.iov_base = read_buf_;
    iov.iov_len = 16;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    bytes_read = recvmsg(socket_fd_, &msg, 0);

    iov.iov_base = read_buf_ + bytes_read;
    iov.iov_len = READ_BUFFER_SIZE - bytes_read;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    bytes_read += recvmsg(socket_fd_, &msg, 0);
#endif

#ifdef REDIS
    bytes_read = read(socket_fd_, read_buf_ + read_idx_, READ_BUFFER_SIZE - read_idx_);
#endif

    read_idx_ += bytes_read;
}

// Writes reponse to socket.
void TCPConn::WriteOnce()
{
    while (true)
    {
        int temp = 0;

#ifdef MEMCACHED
        struct msghdr msg;
        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = iv_;
        msg.msg_iovlen = 1;
        temp = sendmsg(socket_fd_, &msg, 0);
#endif
    
#ifdef NGINX
        temp = writev(socket_fd_, iv_, iv_count_);
#endif

#ifdef MONGODB
        struct msghdr msg;
        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = iv_;
        msg.msg_iovlen = 1;
        temp = sendmsg(socket_fd_, &msg, 0);
#endif

#ifdef REDIS
        temp = write(socket_fd_, write_buf_, bytes_to_send_);
#endif

        bytes_have_send_ += temp;
        bytes_to_send_ -= temp;
        
        if (bytes_to_send_ <= 0)
        {
            Reset();
            return;
        }
    }
}

bool TCPConn::AddResponse(const char *format, ...)
{
    if (write_idx_ >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(write_buf_ + write_idx_, WRITE_BUFFER_SIZE - 1 - write_idx_, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - write_idx_))
    {
        va_end(arg_list);
        return false;
    }
    write_idx_ += len;
    va_end(arg_list);
    return true;
}

bool TCPConn::AddStatusLine(int status, const char *title)
{
    return AddResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool TCPConn::AddHeaders(int content_len)
{
    return AddContentLength(content_len) && AddLinger() && AddBlankLine();
}

bool TCPConn::AddContentLength(int content_len)
{
    return AddResponse("Content-Length:%d\r\n", content_len);
}

bool TCPConn::AddContentType()
{
    return AddResponse("Content-Type:%s\r\n", "text/html");
}

bool TCPConn::AddLinger()
{
    return AddResponse("Connection:%s\r\n", (linger_ == true) ? "keep-alive" : "close");
}

bool TCPConn::AddBlankLine()
{
    return AddResponse("%s", "\r\n");
}

bool TCPConn::AddContent(const char *content)
{
    return AddResponse("%s", content);
}

int TCPConn::ParseRequest()
{
    int request_number = 0;
    int checked_idx = 0;
    char temp;
    for (; checked_idx < read_idx_; ++checked_idx)
    {
        temp = read_buf_[checked_idx];
        if (temp == '\n')
            request_number++;
    }
    return request_number;
}

void TCPConn::Process(uint64_t* mem_data, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs, uint64_t &request_id)
{
    int request_number = ParseRequest();
    request_number = 1;

    for (int i = 0; i < request_number; i++) 
    {
        runAssembly0(mem_data, request_id, curr_mem_addrs, curr_pointer_chasing_mem_addrs);
        request_id++;

#ifdef MEMCACHED
        string res = "reply:Helloworld!Helloworld!Helloworld!";
        const char* response = res.c_str();
        strncpy(write_buf_, response, strlen(response));
        write_idx_ = strlen(response);
        iv_[0].iov_base = write_buf_;
        iv_[0].iov_len = write_idx_;
        iv_count_ = 1;
        bytes_to_send_ = write_idx_;
#endif
    
#ifdef NGINX
        struct stat sb;
        stat("config/index.html", &sb);
        int stat_fd = open("config/index.html", O_RDONLY | O_NONBLOCK);
        if (stat_fd == -1) {
            printf("%s\n", "Nginx cannot open read file.");
        }
        fstat(stat_fd, &sb);
        char tmp_read_buf[2048];
        pread(stat_fd, tmp_read_buf, 612, 0);
        write(nginx_write_fd_, tmp_read_buf, 82);
        close(stat_fd);

        write_idx_ = 238;
        strncpy(write_buf_, tmp_read_buf, write_idx_);
        strncpy(nginx_write_buf_, tmp_read_buf, 612);
        iv_[0].iov_base = write_buf_;
        iv_[0].iov_len = write_idx_;
        iv_[1].iov_base = nginx_write_buf_;
        iv_[1].iov_len = 612;
        iv_count_ = 2;
        bytes_to_send_ = iv_[0].iov_len + iv_[1].iov_len;
#endif

#ifdef MONGODB
        char tmp_read_buf[20480];
        pread(mongod_read_fd_, tmp_read_buf, 20000, 0);
        write(mongod_write_fd_, tmp_read_buf, 8);

        write_idx_ = 1274;
        strncpy(write_buf_, tmp_read_buf, write_idx_);
        iv_[0].iov_base = write_buf_;
        iv_[0].iov_len = write_idx_;
        iv_count_ = 1;
        bytes_to_send_ = iv_[0].iov_len;
#endif

#ifdef REDIS
        write_idx_ = 1250;
        strncpy(write_buf_, redis_read_buf_, write_idx_);
        bytes_to_send_ = write_idx_;
#endif

        WriteOnce();
    }
}