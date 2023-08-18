#ifndef CHATROOM_TCP_H
#define CHATROOM_TCP_H

#include <string>
#include <sys/socket.h>

//这真的就是我们可以用的IP
extern std::string IP;
extern int PORT;

void sys_err(const char *str);

int Socket();

void Listen(int fd, int backlog = 128);

void Bind(int fd, const std::string &ip = IP, int port = PORT);

int Accept(int fd, struct sockaddr *cli_addr, socklen_t *cli_len);

void Connect(int fd, const std::string &ip = IP, int port = PORT);

#endif