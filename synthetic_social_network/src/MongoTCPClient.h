#ifndef AUTO_MICROSERVICES_MONGOTCPCLIENT_H
#define AUTO_MICROSERVICES_MONGOTCPCLIENT_H

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

using namespace std;

namespace auto_microservices {

int hostname_to_ip(const char * hostname , char* ip)
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

class MongoTCPClient {
 public:
  MongoTCPClient(const std::string &addr, int port);

  MongoTCPClient(const MongoTCPClient &) = delete;
  MongoTCPClient &operator=(const MongoTCPClient &) = delete;
  MongoTCPClient &operator=(MongoTCPClient &&) = default;

  ~MongoTCPClient();

  void Send(const std::string &content);
  void Disconnect();

 private:
  int _socket_fd;
  struct iovec _iv;
  char _buff[8192];
};

MongoTCPClient::MongoTCPClient(const std::string &addr, int port) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in server_addr;
  const char *hostname = addr.c_str();
	char ip[100];
  hostname_to_ip(hostname, ip);
  server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

  if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    printf("%s:errno is:%d %s\n", "Connection with the server failed", errno, strerror(errno));
    exit(0); 
  }

  int flag = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));
  _socket_fd = socket_fd;

  int fd = open("config/mongodb.html", O_RDONLY | O_NONBLOCK);
  if (fd == -1) {
    cout << "Mongodb client cannot read file." << endl;
  }
  int count = pread(fd, _buff, 2000, 0);
  _iv.iov_base = _buff;
  _iv.iov_len = 1000;
}

void MongoTCPClient::Send(const std::string &content) {
  struct msghdr msg;
  memset(&msg, 0, sizeof(struct msghdr));
  msg.msg_iov = &_iv;
  msg.msg_iovlen = 1;
  int temp = sendmsg(_socket_fd, &msg, 0);

  // auto start = std::chrono::high_resolution_clock::now();
  char recv_buff[8192];
  int rerr = recv(_socket_fd, recv_buff, 4, 0);
  rerr += recv(_socket_fd, recv_buff, sizeof(recv_buff), 0);
  // auto elapsed = std::chrono::high_resolution_clock::now() - start;
  // long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  // cout << "MongoDB response time: " << microseconds << " microseconds" << endl;
}

MongoTCPClient::~MongoTCPClient() {
  Disconnect();
}

void MongoTCPClient::Disconnect() {
  close(_socket_fd);
}

} // namespace auto_microservices

#endif //AUTO_MICROSERVICES_MONGOTCPCLIENT_H
