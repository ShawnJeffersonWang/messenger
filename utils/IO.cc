//
// Created by shawn on 23-8-7.
//
#include "IO.h"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include "Redis.h"

using namespace std;

int write_n(int fd, const char *msg, int n) {
    int n_written;
    int n_left = n;
    const char *ptr = msg;
    while (n_left > 0) {
        if ((n_written = send(fd, ptr, n_left, 0)) < 0) {
            if (n_written < 0 && errno == EINTR)
                continue;
            else
                return -1;
        } else if (n_written == 0) {
            continue;
        }
        ptr += n_written;
        n_left -= n_written;
    }
    return n;
}

int sendMsg(int fd, string msg) {
    if (fd < 0 || msg.empty()) {
        return -1;
    }
    int ret;
    char *data = (char *) malloc(sizeof(char) * (msg.size() + 4));
    int len = htonl(msg.size());
    memcpy(data, &len, 4);
    memcpy(data + 4, msg.c_str(), msg.size());
    ret = write_n(fd, data, msg.size() + 4);
    if (ret < 0) {
        perror("sendMsg error");
        close(fd);
    }
    return ret;
}

int read_n(int fd, char *msg, int n) {
    int n_left = n;
    int n_read;
    char *ptr = msg;
    while (n_left > 0) {
        //bug recv中的第二个参数写成了msg
        if ((n_read = recv(fd, ptr, n_left, 0)) < 0) {
            //sm bug 导致我的recvMsg直接非阻塞返回-1,进而导致server收到的用户信息直接为空
            if (errno == EINTR || errno == EWOULDBLOCK)
                //continue;
                n_read = 0;
            else
                return -1;
        } else if (n_read == 0)
            break;
        ptr += n_read;
        n_left -= n_read;
    }
    return n - n_left;
}

int recvMsg(int fd, string &msg) {
    int len = 0;
    read_n(fd, (char *) &len, 4);
    len = ntohl(len);
    char *data = (char *) malloc(sizeof(char) * (len + 1));
    int ret = read_n(fd, data, len);
    if (ret == 0) {
        cout << "对端断开连接" << endl;
        close(fd);
    } else if (ret != len) {
        cout << "数据接收失败" << endl;
    }
    data[len] = '\0';
    msg = data;
    return ret;
}