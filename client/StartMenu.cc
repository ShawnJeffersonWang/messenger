//
// Created by shawn on 23-8-7.
//
#include "StartMenu.h"
#include <iostream>
#include <cstdint>
#include "../utils/IO.h"
#include "../utils/proto.h"
#include "OperationMenu.h"
#include "User.h"
#include <termios.h>
#include <unistd.h>

using namespace std;

bool isNumber(const string &input) {
    for (char c: input) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

char getch() {
    char ch;
    struct termios tm, tm_old;
    tcgetattr(STDIN_FILENO, &tm);
    tm_old = tm;
    tm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tm);
    while ((ch = getchar()) == EOF) {
        clearerr(stdin);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &tm_old);
    return ch;
}

//bug 退格仍然输出"*"，且没有退格的效果，是因为终端配置方案中的按键绑定，改为Solaris就好了，这样Backspace才是\b
void get_password(string &password) {
    char ch;
    cout << "请输入您的密码: ";
    while ((ch = getch()) != '\n') {
        if (ch == '\b') {
            if (!password.empty()) {
                cout << "\b \b";
                password.pop_back();
            }
        } else {
            password.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
}

int login(int fd, User &user) {
    sendMsg(fd, LOGIN);
    string UID;
    string passwd;
    while (true) {
        std::cout << "请输入您的UID：" << std::endl;
        getline(cin, UID);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return 0;
        }

        if (UID.length() > 10) {
            cout << "UID长度不能大于10!" << std::endl;
            continue;
        } else if (UID.find(' ') != string::npos) {
            cout << "UID输入不能包含空格" << endl;
            continue;
        }
        break;
    }
    // 密码输入关闭回显
    label:
    get_password(passwd);
    if (passwd.length() > 16) {
        cout << "密码长度不能大于16!" << std::endl;
        passwd.clear();
        goto label;
    } else if (passwd.find(' ') != string::npos) {
        cout << "密码输入不能包含空格" << endl;
        passwd.clear();
        goto label;
    }
    LoginRequest loginRequest(UID, passwd);
    //bug 发过去的是json数组，对方按键值对无法反序列化
    sendMsg(fd, loginRequest.to_json());
    string buf;
    //看服务器让不让你登录
    recvMsg(fd, buf);
    if (buf == "-1") {
        cout << "账号不存在" << endl;
        return 0;
    } else if (buf == "-2") {
        cout << "密码错误" << endl;
        int choice;
        cout << "是否找回密码[1]YES, [0]NO" << endl;
        while (!(cin >> choice)) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return 0;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        if (choice == 0) {
            //取消修改密码
            sendMsg(fd, "Cancel");
            return 0;
        }
        //确认修改密码
        sendMsg(fd, "Confirm");
        findPassword(fd);
    } else if (buf == "-3") {
        //非正常退出会导致数据库中用户还在线上，下次无法登录
        cout << "该用户已经登录" << endl;
        return 0;
    } else if (buf == "1") {
        cout << "登录成功!" << endl;
        string user_info;

        recvMsg(fd, user_info);
        user.json_parse(user_info);
        cout << "好久不见 " << user.getUsername() << "!" << endl;
        return 1;
    }
    return 0;
}

void findPassword(int fd) {
    User user;
    string phoneNumber;
    string reply, once, twice;
    LABEL:
    while (true) {
        cout << "请输入您的手机号" << endl;
        getline(cin, phoneNumber);
        if (cin.eof()) {
            cout << "读到文件末尾" << endl;
            return;
        }

        if (phoneNumber.length() != 11) {
            cout << "手机号长度为11位" << endl;
            continue;
        } else if (phoneNumber.find(' ') != string::npos) {
            cout << "手机号输入不能包含空格" << endl;
            continue;
        }
        break;
    }

    sendMsg(fd, phoneNumber);

    recvMsg(fd, reply);
    if (reply != "Success") {
        cout << "手机号无法匹配" << endl;
        goto LABEL;
    }
    cout << "输入你要修改的新密码" << endl;
    label:
    get_password(once);
    if (once.length() < 4 || once.length() > 16) {
        cout << "密码长度在4到16之间!" << std::endl;
        once.clear();
        goto label;
    } else if (once.find(' ') != string::npos) {
        cout << "密码输入不能包含空格" << endl;
        once.clear();
        goto label;
    }

    cout << "请再次输入" << endl;
    get_password(twice);
    if (once != twice) {
        cout << "两次密码不一致" << endl;
        once.clear();
        twice.clear();
        goto label;
    }

    sendMsg(fd, twice);

    recvMsg(fd, reply);
    if (reply == "Success") {
        cout << "密码修改成功" << endl;
    }
}

void client_register(int fd) {
    sendMsg(fd, REGISTER);
    User user;
    string phoneNumber, username, once, twice;
    while (true) {
        cout << "请输入您的手机号" << endl;
        getline(cin, phoneNumber);
        if (cin.eof()) {
            cout << "读到文件末尾" << endl;
            return;
        }

        if (phoneNumber.length() != 11) {
            cout << "手机号长度为11位" << endl;
            continue;
        } else if (phoneNumber.find(' ') != string::npos) {
            cout << "手机号输入不能包含空格" << endl;
            continue;
        } else if (!isNumber(phoneNumber)) {
            cout << "手机号必须是数字！" << endl;
            continue;
        }
        user.setPhoneNumber(phoneNumber);
        break;
    }
    while (true) {
        std::cout << "请输入您的用户名：" << std::endl;
        getline(cin, username);
//        if (!(cin >> username)) {
//            cout << "读到文件结尾";
//            continue;
//        }
        if (cin.eof()) {
            cout << "读到文件末尾" << endl;
            return;
        }

        if (username.length() < 2 || username.length() > 10) {
            cout << "用户名长度在2到10之间!" << std::endl;
            continue;
        } else if (username.find(' ') != string::npos) {
            cout << "用户名输入不能包含空格" << endl;
            continue;
        }
        user.setUsername(username);
        break;
    }

//    struct termios oldt, newt;
//    tcgetattr(STDIN_FILENO, &oldt);
//    newt = oldt;
//    newt.c_cflag &= ~(ECHO);
    label:
    //std::cout << "请输入您的密码：" << std::endl;
    //tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    //system("stty -echo");
    //使用getline会导致如果用户输入回车直接成功
    //但使用cin也会导致输入如ssd dfsewt直接成功
    //getline(cin, username);
    get_password(once);
//    if (!(cin >> username)) {
//        cout << "输入错误 请重新输入" << endl;
//        goto label;
//    }
    if (once.length() < 4 || once.length() > 16) {
        cout << "密码长度在4到16之间!" << std::endl;
        //bug 没有清空字符串缓冲区,输出*的功能会在字符串留下残留
        once.clear();
        goto label;
    } else if (once.find(' ') != string::npos) {
        cout << "密码输入不能包含空格" << endl;
        once.clear();
        goto label;
    }

    cout << "请再次输入 ";
    //getline(cin, twice);
    get_password(twice);
//    if (!(cin >> username)) {
//        cout << "输入错误 请重新输入" << endl;
//        goto label;
//    }
    //bug 之前由于输入用户名和输入密码使用同一个string username,导致密码比对的时候temp带上了用户名进而导致比对错误
    if (once != twice) {
        cout << "两次密码不一致" << endl;
        once.clear();
        twice.clear();
        goto label;
    }
//    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    //system("stty echo");
    user.setPassword(twice);
    //向服务器发送用户输入的手机号，用户名和密码
    sendMsg(fd, user.to_json());
    string UID;

    recvMsg(fd, UID);
    cout << "账号注册成功！" << endl;
    cout << "您的帐号为：" << UID << endl;
}