#include "Redis.h"
#include "IO.h"
#include "group_chat.h"
#include "Group.h"
#include "proto.h"
//
// Created by shawn on 23-8-13.
//
using namespace std;

GroupChat::GroupChat(int fd, const User &user) : fd(fd), user(user) {
    joined = "joined" + user.getUID();
    created = "created" + user.getUID();
    managed = "managed" + user.getUID();
}

void GroupChat::sync() {
    Redis redis;
    redis.connect();
    int num = redis.scard(created);

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers(created);
        for (int i = 0; i < num; i++) {
            string json = redis.hget("group_info", arr[i]->str);

            sendMsg(fd, json);
            freeReplyObject(arr[i]);
        }
    }
    num = redis.scard(managed);

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers(managed);
        for (int i = 0; i < num; i++) {
            string json = redis.hget("group_info", arr[i]->str);

            sendMsg(fd, json);
            freeReplyObject(arr[i]);
        }
    }
    num = redis.scard(joined);

    sendMsg(fd, to_string(num));
    if (num != 0) {
        redisReply **arr = redis.smembers(joined);
        for (int i = 0; i < num; i++) {
            string json = redis.hget("group_info", arr[i]->str);

            sendMsg(fd, json);
            freeReplyObject(arr[i]);
        }
    }
}

void GroupChat::startChat() {
    Redis redis;
    redis.connect();
    redis.sadd("group_chat", user.getUID());
    string group_info;
    redisReply **arr;

    recvMsg(fd, group_info);
    Group group;
    group.json_parse(group_info);
    int num = redis.llen(group.getGroupUid() + "history");
    if (num < 5) {

        sendMsg(fd, to_string(num));
    } else {
        num = 5;

        sendMsg(fd, to_string(num));
    }
    if (num != 0) {
        arr = redis.lrange(group.getGroupUid() + "history", "0", to_string(num - 1));
    }
    for (int i = num - 1; i >= 0; i--) {

        sendMsg(fd, arr[i]->str);
        freeReplyObject(arr[i]);
    }
    string msg;
    Message message;
    while (true) {
        int ret = recvMsg(fd, msg);
        if (msg == EXIT || ret == 0) {
            sendMsg(fd, EXIT);
            redis.srem("group_chat", user.getUID());
            if (ret == 0) {
                redis.hdel("is_online", user.getUID());
            }
            return;
        }
        message.json_parse(msg);
        int len = redis.scard(group.getMembers());
        if (len == 0) {
            return;
        }

        redis.lpush(group.getGroupUid() + "history", msg);
        message.setUidTo(group.getGroupUid());
        arr = redis.smembers(group.getMembers());
        string UIDto;
        for (int i = 0; i < len; i++) {
            UIDto = arr[i]->str;
            if (UIDto == user.getUID()) {
                freeReplyObject(arr[i]);
                continue;
            }
            if (!redis.hexists("is_online", UIDto)) {
                redis.hset("chat", UIDto, group.getGroupName());
                freeReplyObject(arr[i]);
                continue;
            }
            if (!redis.sismember("group_chat", UIDto)) {
                redis.hset("chat", UIDto, group.getGroupName());
                freeReplyObject(arr[i]);
                continue;
            }
            string s_fd = redis.hget("is_online", UIDto);
            int _fd = stoi(s_fd);
            sendMsg(_fd, msg);
            freeReplyObject(arr[i]);
        }

    }
}

void GroupChat::createGroup() {
    Redis redis;
    redis.connect();
    string group_info;

    recvMsg(fd, group_info);
    Group group;
    group.json_parse(group_info);
    redis.hset("group_info", group.getGroupUid(), group_info);
    redis.sadd(joined, group.getGroupUid());
    redis.sadd(managed, group.getGroupUid());
    redis.sadd(created, group.getGroupUid());
    redis.sadd(group.getMembers(), user.getUID());
    redis.sadd(group.getAdmins(), user.getUID());
}

void GroupChat::joinGroup() {
    Redis redis;
    redis.connect();
    string groupUid;
    //接收客户端发送的群聊UID
    recvMsg(fd, groupUid);
    if (!redis.hexists("group_info", groupUid)) {
        sendMsg(fd, "-1");
        return;
    }
    string json = redis.hget("group_info", groupUid);
    Group group;
    group.json_parse(json);
    //已经加入该群
    if (redis.sismember(joined, groupUid)) {
        sendMsg(fd, "-2");
        return;
    }

    sendMsg(fd, "1");
    redis.sadd("if_add" + groupUid, user.getUID());
    //群聊实时通知
    int num = redis.scard(group.getAdmins());
    redisReply **arr = redis.smembers(group.getAdmins());
    for (int i = 0; i < num; i++) {
        redis.sadd("add_group", arr[i]->str);
    }
}

void GroupChat::groupHistory() const {
    Redis redis;
    redis.connect();
    string group_uid;

    recvMsg(fd, group_uid);
    string group_history = group_uid + "history";
    int num = redis.llen(group_history);

    sendMsg(fd, to_string(num));
    redisReply **arr = redis.lrange(group_history);
    for (int i = num - 1; i >= 0; i--) {

        sendMsg(fd, arr[i]->str);
        freeReplyObject(arr[i]);
    }
}

void GroupChat::managedGroup() const {
    Redis redis;
    redis.connect();
    Group group;
    string group_info;

    recvMsg(fd, group_info);
    group.json_parse(group_info);
    string choice;
    int ret;
    while (true) {

        ret = recvMsg(fd, choice);
        if (ret == 0) {
            redis.hdel("is_online", user.getUID());
        }
        if (choice == BACK) {
            break;
        }
        if (choice == "1") {
            approve(group);
        } else if (choice == "2") {
            remove(group);
        }
    }
}

void GroupChat::approve(Group &group) const {
    Redis redis;
    redis.connect();
    int num = redis.scard("if_add" + group.getGroupUid());

    sendMsg(fd, to_string(num));
    if (num == 0) {
        return;
    }
    redisReply **arr = redis.smembers("if_add" + group.getGroupUid());
    string info;
    string choice;
    User member;
    for (int i = 0; i < num; i++) {
        info = redis.hget("user_info", arr[i]->str);
        member.json_parse(info);

        sendMsg(fd, member.getUsername());

        int ret = recvMsg(fd, choice);
        if (ret == 0) {
            redis.hdel("is_online", user.getUID());
        }
        if (choice == "n") {
            //删除缓冲区
            redis.srem("if_add" + group.getGroupUid(), member.getUID());
        } else {
            redis.sadd("joined" + member.getUID(), group.getGroupUid());
            redis.sadd(group.getMembers(), member.getUID());
            //删除缓冲区
            redis.srem("if_add" + group.getGroupUid(), member.getUID());
        }
        freeReplyObject(arr[i]);
    }
}

void GroupChat::remove(Group &group) const {
    Redis redis;
    redis.connect();
    int num = redis.scard(group.getMembers());
    //发送群员数量
    sendMsg(fd, to_string(num));
    redisReply **arr = redis.smembers(group.getMembers());
    User member;
    string member_info;
    for (int i = 0; i < num; i++) {
        member_info = redis.hget("user_info", arr[i]->str);
        //发送群员信息
        sendMsg(fd, member_info);
        freeReplyObject(arr[i]);
    }
    string remove_info;

    recvMsg(fd, remove_info);
    if (remove_info == "0") {
        return;
    }
    member.json_parse(remove_info);
    GroupChat groupChat(0, member);
    redis.srem(groupChat.joined, group.getGroupUid());
    redis.srem(groupChat.managed, group.getGroupUid());
    redis.srem(group.getMembers(), member.getUID());
    redis.srem(group.getAdmins(), member.getUID());

    redis.sadd(member.getUID() + "del", group.getGroupName());
}

void GroupChat::managedCreatedGroup() const {
    Redis redis;
    redis.connect();
    string group_info;

    recvMsg(fd, group_info);
    Group group;
    group.json_parse(group_info);
    string choice;
    while (true) {

        int ret = recvMsg(fd, choice);
        if (ret == 0) {
            redis.hdel("is_online", user.getUID());
        }
        if (choice == "0") {
            break;
        }
        if (choice == "1") {
            approve(group);
        } else if (choice == "2") {
            revokeAdmin(group);
        } else if (choice == "3") {
            deleteGroup(group);
        }
    }
}

void GroupChat::appointAdmin(Group &group) const {
    Redis redis;
    redis.connect();
    int num = redis.scard(group.getMembers());

    sendMsg(fd, to_string(num));
    string member_info;
    redisReply **arr = redis.smembers(group.getMembers());
    for (int i = 0; i < num; i++) {
        member_info = redis.hget("user_info", arr[i]->str);

        sendMsg(fd, member_info);
        freeReplyObject(arr[i]);
    }
    string member_choose;

    int ret = recvMsg(fd, member_choose);
    if (ret == 0) {
        redis.hdel("is_online", user.getUID());
    }
    User member;
    member.json_parse(member_choose);
    //选择的已经是管理员了
    if (redis.sismember(group.getAdmins(), member.getUID())) {
        sendMsg(fd, "-1");
        return;
    }
    redis.sadd(group.getAdmins(), member.getUID());
    redis.sadd("managed" + member.getUID(), group.getGroupUid());
    //加入缓冲区
    redis.sadd("have_managed" + member.getUID(), group.getGroupName());
    sendMsg(fd, "1");
}

void GroupChat::revokeAdmin(Group &group) const {
    Redis redis;
    redis.connect();
    int num = redis.scard(group.getAdmins());

    sendMsg(fd, to_string(num));
    redisReply **arr = redis.smembers(group.getAdmins());
    string admin_info;
    for (int i = 0; i < num; i++) {
        admin_info = redis.hget("user_info", arr[i]->str);

        sendMsg(fd, admin_info);
        freeReplyObject(arr[i]);
    }
    User admin;

    recvMsg(fd, admin_info);
    admin.json_parse(admin_info);
    redis.srem(group.getAdmins(), admin.getUID());
    redis.srem("managed" + admin.getUID(), group.getGroupUid());
    //缓冲区
    redis.srem("revoked" + admin.getUID(), group.getGroupName());
}

void GroupChat::deleteGroup(Group &group) {
    Redis redis;
    redis.connect();
    int num = redis.scard(group.getMembers());
    redisReply **arr = redis.smembers(group.getMembers());
    string UID;
    for (int i = 0; i < num; i++) {
        UID = arr[i]->str;
        redis.srem("joined" + UID, group.getGroupUid());
        redis.srem("created" + UID, group.getGroupUid());
        redis.srem("managed" + UID, group.getGroupUid());
        freeReplyObject(arr[i]);
    }
    redis.del(group.getMembers());
    redis.del(group.getAdmins());
    redis.del(group.getGroupUid() + "history");
}

void GroupChat::showMembers() const {
    Redis redis;
    redis.connect();
    string group_info;
    Group group;

    recvMsg(fd, group_info);
    group.json_parse(group_info);
    int num = redis.scard(group.getMembers());

    sendMsg(fd, to_string(num));
    redisReply **arr = redis.smembers(group.getMembers());
    User member;
    string member_info;
    for (int i = 0; i < num; i++) {
        member_info = redis.hget("user_info", arr[i]->str);
        member.json_parse(member_info);

        sendMsg(fd, member.getUsername());
        freeReplyObject(arr[i]);
    }
}

void GroupChat::quit() {
    Redis redis;
    redis.connect();
    Group group;
    string group_info;

    recvMsg(fd, group_info);
    group.json_parse(group_info);
    redis.srem("joined" + user.getUID(), group.getGroupUid());
    redis.srem("managed" + user.getUID(), group.getGroupUid());
    redis.srem(group.getMembers(), user.getUID());
    redis.srem(group.getAdmins(), user.getUID());
}
