//
// Created by shawn on 23-8-7.
//
#include "TCP.h"
#include <arpa/inet.h>
#include <cstring>

using namespace std;
string IP;
int PORT;

void sys_err(const char *str) {
    perror(str);
    exit(1);
}

int Socket() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        sys_err("socket error");
    }
    return listenfd;
}

//.h文件中的默认参数值在函数声明中指定
void Listen(int fd, int backlog) {
    int ret = listen(fd, backlog);
    if (ret < 0) {
        sys_err("listen error");
    }
}

void Bind(int fd, const string &ip, int port) {
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &srv_addr.sin_addr.s_addr);
    int ret = bind(fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
    if (ret < 0) {
        sys_err("bind error");
    }
}

int Accept(int fd, struct sockaddr *cli_addr, socklen_t *cli_len) {
    int connfd = accept(fd, cli_addr, cli_len);
    if (connfd < 0) {
        sys_err("accept error");
    }
    return connfd;
}

void Connect(int fd, const std::string &ip, int port) {
    int ret;
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &srv_addr.sin_addr.s_addr);
    ret = connect(fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
    if (ret < 0) {
        sys_err("connect error");
    }
}