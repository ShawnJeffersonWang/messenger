//
// Created by shawn on 23-7-26.
//
#ifndef CHATROOM_REDIS_H
#define CHATROOM_REDIS_H

#include <hiredis/hiredis.h>
#include <string>

using std::string;

//使用前先启动redis-server
class Redis {
public:
    Redis();

    ~Redis();

    void connect();

    void del(const string &key);

    void sadd(const string &key, const string &value);

    bool sismember(const string &key, const string &value);

    int scard(const string &key);

    redisReply **smembers(const string &key);

    void srem(const string &key, const string &value);

    string hget(const string &key, const string &field);

    void hset(const string &key, const string &field, const string &value);

    void hdel(const string &key, const string &field);

    redisReply **hgetall(const string &key);

    int hlen(const string &key);

    bool hexists(const string &key, const string &field);

    int llen(const string &key);

    redisReply **lrange(const string &key, const string &start, const string &stop);

    redisReply **lrange(const string &key);

    void lpush(const string &key, const string &value);

    void ltrim(const string &key);

private:
    redisContext *context;
    redisReply *reply;
};

#endif  // CHATROOM_REDIS_H
