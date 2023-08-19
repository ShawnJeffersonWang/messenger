//
// Created by shawn on 23-8-7.
//
#include "LoginHandler.h"
#include "Transaction.h"
#include <functional>
#include <sys/epoll.h>
#include "../utils/IO.h"
#include "../utils/proto.h"
#include "Redis.h"
#include <iostream>
#include <map>
#include <unistd.h>

using namespace std;

void serverLogin(int epfd, int fd) {
    struct epoll_event temp;
    temp.data.fd = fd;
    temp.events = EPOLLIN;
    User user;
    Redis redis;
    redis.connect();
    string request;
    //接收用户发送的UID和密码
    recvMsg(fd, request);
    LoginRequest loginRequest;
    loginRequest.json_parse(request);
    //得到用户发送的UID和密码
    string UID = loginRequest.getUID();
    string passwd = loginRequest.getPasswd();
    if (!redis.sismember("all_uid", UID)) {
        //账号不存在
        sendMsg(fd, "-1");
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &temp);
        return;
    }
    //bug User类序列化后的json数据，中的时间带了空格，导致redis认为参数过多
    string user_info = redis.hget("user_info", UID);
    user.json_parse(user_info);
    if (passwd != user.getPassword()) {
        //密码错误
        sendMsg(fd, "-2");
        string isFindPasswd;

        recvMsg(fd, isFindPasswd);
        if (isFindPasswd == "Confirm") {
            //这是在用户UID正确的情况下，不需要用户再输入UID找回密码
            findPassword(fd, UID);
        }
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &temp);
        return;
    }
    //用户已经登录
    if (redis.hexists("is_online", UID)) {
        sendMsg(fd, "-3");
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &temp);
        return;
    }
    //登录成功
    sendMsg(fd, "1");
    redis.hset("is_online", UID, to_string(fd));
    //发送从数据库获取的用户信息
    sendMsg(fd, user_info);
    serverOperation(fd, user);
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &temp);
}

void findPassword(int fd, const string &UID) {
    Redis redis;
    redis.connect();
    string phoneNumber;
    User user;

    string passwd;
    while (true) {
        recvMsg(fd, phoneNumber);
        string user_info = redis.hget("user_info", UID);
        user.json_parse(user_info);
        if (phoneNumber != user.getPhoneNumber()) {
            sendMsg(fd, "Failed");
            continue;
        }
        break;
    }

    sendMsg(fd, "Success");

    recvMsg(fd, passwd);
    user.setPassword(passwd);
    redis.hset("user_info", UID, user.to_json());
    sendMsg(fd, "Success");
}

void serverRegister(int epfd, int fd) {
    struct epoll_event temp;
    temp.data.fd = fd;
    temp.events = EPOLLIN;
    string user_info;
    //bug recvMsg中的read_n问题，没有设置遇到EWOULDBLOCK继续读，导致user_info读到的为空
    //接收用户注册时发送的帐号密码
    recvMsg(fd, user_info);
    //cout << "user_info:" << user_info << endl;
    User user;
    user.json_parse(user_info);
    string UID = user.getUID();
    //构造函数对成员变量初始化才没有警告
    Redis redis;
    //bug：数据库连接问题,redis-server没有启动
    redis.connect();
    //bug： 数据库添加失败,json数据带有空格
    redis.hset("user_info", UID, user_info);
    redis.sadd("all_uid", UID);

    sendMsg(fd, UID);
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &temp);
}

void serverOperation(int fd, User &user) {
    map<int, function<void(int fd, User &user)>> LoginOperation = {
            {4,  start_chat},
            {5,  history},
            {6,  list_friend},
            {7,  add_friend},
            {8,  findRequest},
            {9,  del_friend},
            {10, blockedLists},
            {11, unblocked},
            {12, group},
            {13, send_file},
            {14, receive_file},
            //改协议后，同步出错
            {17, synchronize}
    };
    Redis redis;
    redis.connect();
    int friend_num = redis.scard(user.getUID());
    //发送好友个数
    sendMsg(fd, to_string(friend_num));
    redisReply **arr = redis.smembers(user.getUID());
    for (int i = 0; i < friend_num; i++) {
        string friend_info = redis.hget("user_info", arr[i]->str);
        //循环发送好友信息
        sendMsg(fd, friend_info);
    }
    string temp;
    int ret;
    while (true) {
        //接收用户输入的操作
        //important: ret 看来是必要的 可以有效的删掉在线用户，防止虚空在线
        ret = recvMsg(fd, temp);
        if (temp == BACK || ret == 0) {
            break;
        }
        int option = stoi(temp);
        if (LoginOperation.find(option) == LoginOperation.end()) {
            cout << "没有这个选项，请重新输入" << endl;
            continue;
        }
        LoginOperation[option](fd, user);
    }
    close(fd);
    //只用在这里删除就行了
    redis.hdel("is_online", user.getUID());
}

void notify(int fd) {
    Redis redis;
    redis.connect();
    string UID;

    int ret = recvMsg(fd, UID);
    if (ret == 0) {
        redis.hdel("is_online", UID);
    }
    //判断是否有好友添加
    if (redis.sismember("add_friend", UID)) {

        sendMsg(fd, REQUEST_NOTIFICATION);
        redis.srem("add_friend", UID);
    } else {

        sendMsg(fd, "NO");
    }
    //判断是否有加群
    if (redis.sismember("add_group", UID)) {

        sendMsg(fd, GROUP_REQUEST);
        redis.srem("add_group", UID);
    } else {

        sendMsg(fd, "NO");
    }
    //判断是否有消息
    if (redis.hexists("chat", UID)) {

        sendMsg(fd, redis.hget("chat", UID));
        //bug hdel写的有问题，返回的一直是"0",key和field之间没有加空格,并且hdel还写错了
        redis.hdel("chat", UID);
    } else {

        sendMsg(fd, "NO");
    }
    //被删除好友
    int num = redis.scard(UID + "del");

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers(UID + "del");
        for (int i = 0; i < num; i++) {

            sendMsg(fd, arr[i]->str);
            redis.srem(UID + "del", arr[i]->str);
            freeReplyObject(arr[i]);
        }
    }
    //被设置为管理员
    num = redis.scard("appoint_admin" + UID);

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers("appoint_admin" + UID);
        for (int i = 0; i < num; i++) {

            sendMsg(fd, arr[i]->str);
            redis.srem("appoint_admin" + UID, arr[i]->str);
            freeReplyObject(arr[i]);
        }
    }
    //被取消管理员权限
    num = redis.scard("revoke_admin" + UID);

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers("revoke_admin" + UID);
        for (int i = 0; i < num; i++) {

            sendMsg(fd, arr[i]->str);
            redis.srem("revoke_admin" + UID, arr[i]->str);
            freeReplyObject(arr[i]);
        }
    }
    //文件消息提醒
    num = redis.scard("file" + UID);

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers("file" + UID);
        for (int i = 0; i < num; i++) {

            sendMsg(fd, arr[i]->str);
            redis.srem("file" + UID, arr[i]->str);
            freeReplyObject(arr[i]);
        }
    }
}