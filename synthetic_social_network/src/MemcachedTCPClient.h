#ifndef AUTO_MICROSERVICES_MEMCACHEDTCPCLIENT_H
#define AUTO_MICROSERVICES_MEMCACHEDTCPCLIENT_H

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <chrono>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/tcp.h>

using namespace std;

namespace auto_microservices {

int memcached_hostname_to_ip(const char * hostname , char* ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	if((he = gethostbyname(hostname)) == NULL) 
	{
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	return 1;
}

class MemcachedTCPClient {
 public:
  MemcachedTCPClient(const std::string &addr, int port);

  MemcachedTCPClient(const MemcachedTCPClient &) = delete;
  MemcachedTCPClient &operator=(const MemcachedTCPClient &) = delete;
  MemcachedTCPClient &operator=(MemcachedTCPClient &&) = default;

  ~MemcachedTCPClient();

  void Send(const std::string &content);
  void Disconnect();

 private:
  struct iovec _iv;
  char _buff[8192];
  char _ip[100];
  int _port;
};

MemcachedTCPClient::MemcachedTCPClient(const std::string &addr, int port) {
  const char *hostname = addr.c_str();
  memcached_hostname_to_ip(hostname, _ip);
  _port = port;

  int fd = open("config/memcached.html", O_RDONLY | O_NONBLOCK);
  pread(fd, _buff, 2000, 0);
  _iv.iov_base = _buff;
  _iv.iov_len = 1000;
}

void MemcachedTCPClient::Send(const std::string &content) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(_ip);
	server_addr.sin_port = htons(_port);

  if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    printf("%s:errno is:%d %s\n", "Connection with the server failed", errno, strerror(errno));
    exit(0); 
  }

  struct linger tmp = {1, 1};
  setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
  int flag = 1;
  setsockopt(socket_fd, SOL_TCP, SO_DEBUG, &flag, sizeof(flag));

  int err = send(socket_fd, _buff, 40, 0);
  err += send(socket_fd, _buff, 40, 0);
  err += send(socket_fd, _buff, 40, 0);

  char recv_buff[8192];
  int rerr = recv(socket_fd, recv_buff, 4, 0);
  rerr += recv(socket_fd, recv_buff, sizeof(recv_buff), 0);

  shutdown(socket_fd, SHUT_WR);
  shutdown(socket_fd, SHUT_RD);
  close(socket_fd);
}

MemcachedTCPClient::~MemcachedTCPClient() {
  Disconnect();
}

void MemcachedTCPClient::Disconnect() {
  // close(_socket_fd);
}

} // namespace auto_microservices

#endif //AUTO_MICROSERVICES_MEMCACHEDTCPCLIENT_H
