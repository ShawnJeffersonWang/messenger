//
// Created by shawn on 23-8-7.
//
#include "Redis.h"
#include <iostream>

using namespace std;

//server的所有代码的核心都落在数据库的操作上，使用数据库的数据，入库
Redis::Redis() : context(nullptr), reply(nullptr) {}

//smbug 之前没写析构函数，我真是nm艹了，怎么想的到是redis的问题，redis一直在开文件描述符，导致服务器和客户端都崩了
Redis::~Redis() {
    redisFree(context);
    context = nullptr;
    reply = nullptr;
}

void Redis::connect() {
    context = redisConnect("127.0.0.1", 6379);
    if (context == nullptr || context->err) {
        if (context->err)
            cout << context->errstr << endl;
        else
            cout << "can't allocate redis context" << endl;
    }
}

void Redis::del(const string &key) {
    string command = "del " + key;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

void Redis::sadd(const string &key, const string &value) {
    string command = "SADD " + key + " " + value;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

bool Redis::sismember(const string &key, const string &value) {
    string command = "SISMEMBER " + key + " " + value;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    bool is_member = reply->integer > 0;
    freeReplyObject(reply);
    return is_member;
}

int Redis::scard(const string &key) {
    string command = "SCARD " + key;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    int integer = reply->integer;
    freeReplyObject(reply);
    return integer;
}

redisReply **Redis::smembers(const string &key) {
    string command = "SMEMBERS " + key;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    return reply->element;
}

void Redis::srem(const string &key, const string &value) {
    string command = "SREM " + key + " " + value;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

string Redis::hget(const string &key, const string &field) {
    string command = "HGET " + key + " " + field;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    //bug 之前在free之后直接返回reply->str,导致直接没反应
    string get_info = reply->str;
    freeReplyObject(reply);
    return get_info;
}

void Redis::hset(const string &key, const string &field, const string &value) {
    string command = "HSET " + key + " " + field + " " + value;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

void Redis::hdel(const string &key, const string &field) {
    //我nm真是艹了 key和field之间没有加空格
    //bug2 并且nm写成了HEDL
    string command = "HDEL " + key + " " + field;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

bool Redis::hexists(const string &key, const string &field) {
    string command = "HEXISTS " + key + " " + field;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    bool exist = reply->integer > 0;
    freeReplyObject(reply);
    return exist;
}

int Redis::llen(const string &key) {
    string command = "LLEN " + key;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    int integer = reply->integer;
    freeReplyObject(reply);
    return integer;
}

redisReply **Redis::lrange(const string &key, const string &start, const string &stop) {
    string command = "LRANGE " + key + " " + start + " " + stop;
    reply = static_cast<redisReply *>(redisCommand(context, command.data()));
    return reply->element;
}

redisReply **Redis::lrange(const string &key) {
    string command = "LRANGE " + key + " 0" + " -1";
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    return reply->element;
}

void Redis::lpush(const string &key, const string &value) {
    string command = "LPUSH " + key + " " + value;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

void Redis::ltrim(const string &key) {
    string command = "LTRIM " + key + " 1 0";
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    freeReplyObject(reply);
}

redisReply **Redis::hgetall(const string &key) {
    string command = "HGETALL " + key;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    return reply->element;
}

int Redis::hlen(const string &key) {
    string command = "HLEN " + key;
    reply = static_cast<redisReply *>(redisCommand(context, command.c_str()));
    int integer = reply->integer;
    freeReplyObject(reply);
    return integer;
}
