//
// Created by shawn on 23-8-7.
//
#include "proto.h"
#include <json/json.h>

#include <utility>
//数据传输操作上方空一行以突出逻辑
using namespace std;

LoginRequest::LoginRequest() = default;

//LoginRequest(const std::string &uid, const std::string &passwd) : UID(uid), passwd(passwd) {}
LoginRequest::LoginRequest(std::string uid, std::string passwd) : UID(std::move(uid)), passwd(std::move(passwd)) {}

[[nodiscard]] const string &LoginRequest::getUID() const {
    return UID;
}

void LoginRequest::setUid(const std::string &uid) {
    UID = uid;
}

[[nodiscard]] const string &LoginRequest::getPasswd() const {
    return passwd;
}

void LoginRequest::setPasswd(const std::string &password) {
    passwd = password;
}

string LoginRequest::to_json() {
    Json::Value root;
    //bug 序列化和反序列化方式不不一致，一种是数组，一种是对象键值对
    //root.append(UID);
    root["UID"] = UID;
    root["passwd"] = passwd;
    Json::FastWriter writer;
    return writer.write(root);
}

void LoginRequest::json_parse(const string &json) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(json, root);
    UID = root["UID"].asString();
    passwd = root["passwd"].asString();
}


Message::Message() = default;

Message::Message(string username, string UID_from, string UID_to, string groupName) : username(std::move(username)),
                                                                                      UID_from(std::move(UID_from)),
                                                                                      UID_to(std::move(UID_to)),
                                                                                      group_name(
                                                                                              std::move(groupName)) {}

[[nodiscard]] string Message::getUsername() const {
    return username;
}

void Message::setUsername(const string &name) {
    username = name;
}

[[nodiscard]] const string &Message::getUidFrom() const {
    return UID_from;
}

void Message::setUidFrom(const string &uidFrom) {
    UID_from = uidFrom;
}

[[nodiscard]] const string &Message::getUidTo() const {
    return UID_to;
}

void Message::setUidTo(const string &uidTo) {
    UID_to = uidTo;
}

[[nodiscard]] string Message::getContent() const {
    return content;
}

void Message::setContent(const string &msg) {
    content = msg;
}

[[nodiscard]] const string &Message::getGroupName() const {
    return group_name;
}

void Message::setGroupName(const string &groupName) {
    group_name = groupName;
}

[[nodiscard]] string Message::getTime() const {
    return timeStamp;
}

void Message::setTime(const string &t) {
    timeStamp = t;
}

string Message::get_time() {
    time_t raw_time;
    struct tm *ptm;
    time(&raw_time);
    ptm = localtime(&raw_time);
    //char* data=new char[60];
    unique_ptr<char[]> data(new char[60]);
    sprintf(data.get(), "%d-%d-%d-%02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour,
            ptm->tm_min, ptm->tm_sec);
    //string result(data);
    //delete[] data;
    string result(data.get());
    return result;
}

string Message::to_json() {
    //bug 消息时间time没有初始化
    //变量名time与get_time中的time函数冲突，现改名为timeStamp
    timeStamp = get_time();
    Json::Value root;
    root["timeStamp"] = timeStamp;
    root["username"] = username;
    root["UID_from"] = UID_from;
    root["UID_to"] = UID_to;
    root["content"] = content;
    root["group_name"] = group_name;
    Json::FastWriter writer;
    return writer.write(root);
}

void Message::json_parse(const string &json) {
    Json::Value root;
    Json::Reader reader;
    reader.parse(json, root);
    timeStamp = root["timeStamp"].asString();
    username = root["username"].asString();
    UID_from = root["UID_from"].asString();
    UID_to = root["UID_to"].asString();
    content = root["content"].asString();
    group_name = root["group_name"].asString();
}
