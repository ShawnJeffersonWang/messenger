//
// Created by shawn on 23-7-27.
//
#ifndef CHATROOM_USER_H
#define CHATROOM_USER_H

#include <string>
//CMakeLists.txt中没有在client.out中加上User.cc, 导致编译client的时候链接错误
using std::string;

class User {

public:
    User();

    static string get_time();

    void setUID(string uid);

    [[nodiscard]] string getUID() const;

    void setPassword(string password);

    void setUsername(string name);

    [[nodiscard]] string getMyTime() const;

    [[nodiscard]] string getUsername() const;

    void setPhoneNumber(string number);

    [[nodiscard]] string getPassword() const;

    void setIsOnline(bool online);

    [[nodiscard]] bool getIsOnline() const;

    string to_json();

    //bug 参数里不能加&
    void json_parse(const string &json);

    [[nodiscard]] const string &getPhoneNumber() const;

private:
    string UID;    //由注册时间关联，10位
    string phone_number;      //手机号，用于密码找回
    string username; //用户名
    string passwd; //密码
    string my_time;  //该用户创建时间
    bool is_online;//是否咋在线;false代表不在线，非0代表在fd
};

#endif //CHATROOM_USER_H
