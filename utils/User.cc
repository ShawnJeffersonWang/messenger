//
// Created by shawn on 23-8-7.
//
#include <random>
#include <iostream>
#include "User.h"
#include "json/json.h"

using namespace std;

//现将get_time封装到User类中，作为User的一部分使用，更具整体性
string User::get_time() {
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


User::User() : is_online(false) {
    random_device rd;
    mt19937 eng(rd());
    uniform_int_distribution<> dist(10, 99);
    int random_num = dist(eng);

    time_t timer;
    time(&timer);
    //cout<<"timer: "<<to_string(timer)<<endl;
    string timeStamp = to_string(timer).substr(8, 2);
    //cout<<"timeStamp: "<<timeStamp<<endl;
    UID = to_string(random_num).append(timeStamp);
    my_time = get_time();
    passwd = "";
    username = "";
}

void User::setUID(string uid) {
    UID = std::move(uid);
}

[[nodiscard]] string User::getUID() const {
    return UID;
}

void User::setPassword(string password) {
    passwd = std::move(password);
}

void User::setUsername(string name) {
    username = std::move(name);
}

string User::getMyTime() const {
    return my_time;
}

[[nodiscard]] string User::getUsername() const {
    return username;
}

void User::setPhoneNumber(string number) {
    phone_number = std::move(number);
}

[[nodiscard]] string User::getPassword() const {
    return passwd;
}

void User::setIsOnline(bool online) {
    is_online = online;
}

[[nodiscard]] bool User::getIsOnline() const {
    return is_online;
}

string User::to_json() {
    Json::Value root;
//        root.append(UID);
//        root.append(passwd);
//        root.append(username);
//        root.append(my_time);
//        root.append(phone_number);
    root["my_time"] = my_time;
    root["UID"] = UID;
    root["username"] = username;
    root["passwd"] = passwd;
    root["phone_number"] = phone_number;
    Json::FastWriter writer;
    return writer.write(root);
}

//bug 参数里不能加&
void User::json_parse(const string &json) {
    Json::Value root;
    Json::Reader reader;
    reader.parse(json, root);
//        int i=0;
//        UID=root[i++].asString();
//        passwd=root[i++].asString();
//        username=root[i++].asString();
//        my_time=root[i++].asString();
//        phone_number=root[i++].asString();
    my_time = root["my_time"].asString();
    UID = root["UID"].asString();
    username = root["username"].asString();
    passwd = root["passwd"].asString();
    phone_number = root["phone_number"].asString();
}

const string &User::getPhoneNumber() const {
    return phone_number;
}
