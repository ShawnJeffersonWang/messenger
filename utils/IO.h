#ifndef CHATROOM_IO_H
#define CHATROOM_IO_H

#include <string>

int write_n(int fd, const char *msg, int n);

int sendMsg(int fd, std::string msg);

int read_n(int fd, char *msg, int n);

int recvMsg(int fd, std::string &msg);

#endif
