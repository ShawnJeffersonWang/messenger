//
// Created by shawn on 23-8-7.
//
#include "Transaction.h"
#include "Redis.h"
#include "../utils/IO.h"
#include "../utils/proto.h"
#include <iostream>
#include "group_chat.h"
#include <functional>
#include <map>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>

using namespace std;

void synchronize(int fd, User &user) {
    Redis redis;
    redis.connect();
    string friend_info;
    int num = redis.scard(user.getUID());
    //向客户端发送好友数量
    sendMsg(fd, to_string(num));
    redisReply **arr = redis.smembers(user.getUID());
    for (int i = 0; i < num; i++) {
        friend_info = redis.hget("user_info", arr[i]->str);

        sendMsg(fd, friend_info);
        freeReplyObject(arr[i]);
    }
}

void start_chat(int fd, User &user) {
    Redis redis;
    redis.connect();
    redis.sadd("is_chat", user.getUID());
    string records_index;
    //接收历史记录索引
    recvMsg(fd, records_index);
    int num = redis.llen(records_index);
    if (num <= 10) {
        //发送历史记录数
        sendMsg(fd, to_string(num));
    } else {
        //最多显示10条
        num = 10;
        sendMsg(fd, "10");
    }
    //bug 只写key的话就是全部发，但是不能全部发
    redisReply **arr = redis.lrange(records_index, "0", to_string(num - 1));
    //先发最新的消息，所以要倒序遍历
    for (int i = num - 1; i >= 0; i--) {

        sendMsg(fd, arr[i]->str);
        freeReplyObject(arr[i]);
    }
    string friend_uid;
    //接收客户端发送的想要聊天的好友的UID，判断是否拉黑的逻辑
    recvMsg(fd, friend_uid);
    if (redis.sismember("blocked" + friend_uid, user.getUID())) {
        sendMsg(fd, "-1");
        return;
    }
    sendMsg(fd, "1");
    string msg;
    while (true) {
        int ret = recvMsg(fd, msg);
        if (msg == EXIT || ret == 0) {
            //给线程发消息
            sendMsg(fd, EXIT);
            redis.srem("is_chat", user.getUID());
            //用户异常退出直接删除在线列表
            if (ret == 0) {
                redis.hdel("is_online", user.getUID());
            }
            return;
        }
        Message message;
        message.json_parse(msg);
        string UID = message.getUidTo();
//        if (redis.sismember("blocked" + UID, user.getUID())) {
//            continue;
//        }
        //对方不在线
        if (!redis.hexists("is_online", UID)) {
            redis.hset("chat", UID, message.getUsername());
            string me = message.getUidFrom() + message.getUidTo();
            string her = message.getUidTo() + message.getUidFrom();
            redis.lpush(me, msg);
            redis.lpush(her, msg);
            continue;
        }
        //对方不在聊天
        if (!redis.sismember("is_chat", UID)) {
            redis.hset("chat", UID, message.getUsername());
            string me = message.getUidFrom() + message.getUidTo();
            string her = message.getUidTo() + message.getUidFrom();
            redis.lpush(me, msg);
            redis.lpush(her, msg);
            continue;
        }
        string _fd = redis.hget("is_online", UID);
        int her_fd = stoi(_fd);
        //cout<<"fd: "<<fd<<endl;
        sendMsg(her_fd, msg);
        string me = message.getUidFrom() + message.getUidTo();
        string her = message.getUidTo() + message.getUidFrom();
        redis.lpush(me, msg);
        redis.lpush(her, msg);
    }
}

void history(int fd, User &user) {
    Redis redis;
    redis.connect();
    string UID;
    //接收客户端发送的好友UID查找历史记录
    recvMsg(fd, UID);
    string temp = UID + user.getUID();
    cout << temp << endl;
    int num = redis.llen(temp);
    //发送历史记录数量
    sendMsg(fd, to_string(num));
    redisReply **arr = redis.lrange(temp);
    //倒序遍历，先发送最新的聊天记录
    for (int i = num - 1; i >= 0; i--) {
        //循环发送历史记录
        sendMsg(fd, arr[i]->str);
        freeReplyObject(arr[i]);
    }
}

void list_friend(int fd, User &user) {
    Redis redis;
    redis.connect();
    string temp;

    recvMsg(fd, temp);
    int num = stoi(temp);
    string friend_uid;
    for (int i = 0; i < num; ++i) {
        //接收客户端发送的UID来查询好友是否在线的信息
        recvMsg(fd, friend_uid);
        if (redis.hexists("is_online", friend_uid)) {
            //在线发送"1"
            sendMsg(fd, "1");
        } else
            //不在线发送"0"
            sendMsg(fd, "0");
    }
}

void add_friend(int fd, User &user) {
    Redis redis;
    redis.connect();
    string UID;

    recvMsg(fd, UID);
    if (!redis.hexists("user_info", UID)) {
        sendMsg(fd, "-1");
        return;
        //判断是否在我的好友列表里
    } else if (redis.sismember(user.getUID(), UID)) {
        sendMsg(fd, "-2");
        return;
    } else if (UID == user.getUID()) {
        //判断添加的是不是自己
        sendMsg(fd, "-3");
        return;
    }
    //要添加的好友存在
    sendMsg(fd, "1");
    //加到实时通知缓冲区中
    redis.sadd("add_friend", UID);
    //加到对方的好友申请的缓冲区中
    redis.sadd(UID + "add_friend", user.getUID());
    string user_info;
    user_info = redis.hget("user_info", UID);

    sendMsg(fd, user_info);
}

void findRequest(int fd, User &user) {
    Redis redis;
    redis.connect();
    //只是一个好友申请缓冲区
    int num = redis.scard(user.getUID() + "add_friend");
    //发送缓冲区申请数量
    sendMsg(fd, to_string(num));
    if (num == 0) {
        return;
    }
    redisReply **arr = redis.smembers(user.getUID() + "add_friend");
    string request_info;
    User friendRequest;
    for (int i = 0; i < num; i++) {
        request_info = redis.hget("user_info", arr[i]->str);
        friendRequest.json_parse(request_info);

        sendMsg(fd, friendRequest.getUsername());
        string reply;

        recvMsg(fd, reply);
        if (reply == "REFUSED") {
            redis.srem(user.getUID() + "add_friend", arr[i]->str);
            return;
        }
        //这里才是真正的好友列表
        //将对方加到我的好友列表中
        redis.sadd(user.getUID(), arr[i]->str);
        //将我加到对方的好友列表中
        redis.sadd(arr[i]->str, user.getUID());
        //将好友申请从缓冲区删除
        redis.srem(user.getUID() + "add_friend", arr[i]->str);

        sendMsg(fd, request_info);
        freeReplyObject(arr[i]);
    }
}

void del_friend(int fd, User &user) {
    Redis redis;
    redis.connect();
    string UID;

    recvMsg(fd, UID);
    //从我的好友列表删除对方
    redis.srem(user.getUID(), UID);
    //在对方的好友列表删除我
    redis.srem(UID, user.getUID());
    //删除历史记录
    redis.ltrim(user.getUID() + UID);
    redis.ltrim(UID + user.getUID());
    //删除黑名单屏蔽
    redis.srem("blocked" + user.getUID(), UID);
    redis.srem("blocked" + UID, user.getUID());
    //缓冲区，用来通知对面被我删除
    redis.sadd(UID + "del", user.getUsername());
}

void blockedLists(int fd, User &user) {
    Redis redis;
    redis.connect();
    User blocked;
    string blocked_uid;
    //接收客户端发送的要屏蔽用户的UID
    recvMsg(fd, blocked_uid);
    redis.sadd("blocked" + user.getUID(), blocked_uid);
}

void unblocked(int fd, User &user) {
    Redis redis;
    redis.connect();
    int num = redis.scard("blocked" + user.getUID());
    //发送屏蔽名单数量
    sendMsg(fd, to_string(num));
    if (num == 0) {
        return;
    }
    redisReply **arr = redis.smembers("blocked" + user.getUID());
    string blocked_info;
    for (int i = 0; i < num; ++i) {
        blocked_info = redis.hget("user_info", arr[i]->str);
        //循环发送屏蔽名单信息
        sendMsg(fd, blocked_info);
        freeReplyObject(arr[i]);
    }
    //接收解除屏蔽的信息
    string UID;
    recvMsg(fd, UID);
    redis.srem("blocked" + user.getUID(), UID);
}

void group(int fd, User &user) {
    Redis redis;
    redis.connect();
    GroupChat groupChat(fd, user);
    groupChat.sync();
    string choice;
    map<int, function<void()>> groupOperation = {
            {1,  [groupChat]() mutable { groupChat.startChat(); }},
            {2,  [groupChat]() mutable { groupChat.createGroup(); }},
            {3,  [groupChat]() mutable { groupChat.joinGroup(); }},
            {4,  [groupChat]() mutable { groupChat.groupHistory(); }},
            {5,  [groupChat]() mutable { groupChat.managedGroup(); }},
            {6,  [groupChat]()mutable { groupChat.managedCreatedGroup(); }},
            {7,  [groupChat]() mutable { groupChat.showMembers(); }},
            {8,  [groupChat]() mutable { groupChat.quit(); }},
            {11, [groupChat]()mutable { groupChat.sync(); }}
    };
    int ret;
    while (true) {

        ret = recvMsg(fd, choice);
        if (ret == 0) {
            redis.hdel("is_online", user.getUID());
        }
        if (choice == BACK) {
            break;
        }
        int option = stoi(choice);
        if (groupOperation.find(option) == groupOperation.end()) {
            continue;
        }
        groupOperation[option]();
    }
}

void send_file(int fd, User &user) {
    Redis redis;
    redis.connect();
    string friend_info;
    //接收发送文件的对象
    User _friend;
    int ret = recvMsg(fd, friend_info);
    if (ret == 0) {
        redis.hdel("is_online", user.getUID());
    }
    if (friend_info == BACK) {
        return;
    }
    _friend.json_parse(friend_info);
    string filePath, fileName;

    ret = recvMsg(fd, filePath);
    if (ret == 0) {
        redis.hdel("is_online", user.getUID());
    }

    recvMsg(fd, fileName);
    cout << "传输文件名: " << fileName << endl;
    filePath = "./fileBuffer/" + fileName;
    //最后一个groupName填了文件名，接收的时候会显示
    Message message(user.getUsername(), user.getUID(), _friend.getUID(), fileName);
    message.setContent(filePath);
    if (!filesystem::exists("./fileBuffer")) {
        filesystem::create_directories("./fileBuffer");
    }
    ofstream ofs(filePath, ios::binary);
    if (!ofs.is_open()) {
        cerr << "Can't open file" << endl;
        return;
    }
    string ssize;

    recvMsg(fd, ssize);
    off_t size = stoll(ssize);
    off_t sum = 0;
    int n;
    char buf[BUFSIZ];
    while (size > 0) {
        if (size > sizeof(buf)) {
            n = read_n(fd, buf, sizeof(buf));
        } else {
            n = read_n(fd, buf, size);
        }
        if (n < 0) {
            continue;
        }
        cout << "剩余文件大小: " << size << endl;
        size -= n;
        sum += n;
        ofs.write(buf, n);
    }
    //文件发送实时通知缓冲区
    redis.sadd("file" + _friend.getUID(), user.getUsername());
    redis.sadd("recv" + _friend.getUID(), message.to_json());
    ofs.close();
}

void receive_file(int fd, User &user) {
    Redis redis;
    redis.connect();
    char buf[BUFSIZ];
    int num = redis.scard("recv" + user.getUID());

    sendMsg(fd, to_string(num));
    Message message;
    string path;
    if (num == 0) {
        cout << "当前没有要接收的文件" << endl;
        return;
    }

    redisReply **arr = redis.smembers("recv" + user.getUID());
    for (int i = 0; i < num; i++) {

        sendMsg(fd, arr[i]->str);
        message.json_parse(arr[i]->str);
        path = message.getContent();
        struct stat info;
        if (stat(path.c_str(), &info) == -1) {
            cout << "非法的路径名" << endl;
            cout << path.c_str() << endl;
            return;
        }
        string reply;

        int _ret = recvMsg(fd, reply);
        if (_ret == 0) {
            redis.hdel("is_online", user.getUID());
        }
        if (reply == "NO") {
            cout << "拒接接收文件" << endl;
            redis.srem("recv" + user.getUID(), arr[i]->str);
            freeReplyObject(arr[i]);
            continue;
        }

        int fp = open(path.c_str(), O_RDONLY);

        sendMsg(fd, to_string(info.st_size));
        off_t ret;
        off_t sum = info.st_size;
        off_t size = 0;
        while (true) {
            ret = sendfile(fd, fp, nullptr, info.st_size);
            if (ret == 0) {
                cout << "文件传输成功" << buf << endl;
                break;
            } else if (ret > 0) {
                cout << ret << endl;
                sum -= ret;
                size += ret;
            }
        }
        redis.srem("recv" + user.getUID(), arr[i]->str);
        close(fp);
        freeReplyObject(arr[i]);
    }
}