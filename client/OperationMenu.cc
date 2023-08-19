//
// Created by shawn on 23-8-7.
//
#include "OperationMenu.h"
#include "Telegram.h"
#include "Notifications.h"
#include <functional>
#include <map>
#include <iostream>
#include <thread>
#include <iomanip>
#include "../utils/proto.h"
#include "../utils/IO.h"

using namespace std;

void clientOperation(int fd, User &user) {
    string my_uid = user.getUID();
    User _friend;
    string friend_num;
    //接收好友个数
    recvMsg(fd, friend_num);
    int num = stoi(friend_num);
    cout << "您的好友个数为:" << num << endl;
    //使用vector方便使用索引
    //之后传给各个函数传来传去的就是这个好友列表
    vector<pair<string, User>> my_friends;
    //将好友存储到my_friends中
    string friend_info;
    for (int i = 0; i < num; i++) {
        //循环接收好友信息
        recvMsg(fd, friend_info);
        _friend.json_parse(friend_info);
        //前面填UID代表是谁的好友列表
        my_friends.emplace_back(my_uid, _friend);
    }
    Telegram telegram(fd, user);
    //bug 使用ref就不用bind了
    //bug 找了一下午就是这个线程的问题,现在改为不引用&
    thread work(announce, user.getUID());
    work.detach();
    //成员函数指针初始化 std::function 对象
    map<int, std::function<void(vector<pair<string, User>> &)>> FunctionOption = {
            {1,  bind(&Telegram::startChat, telegram, placeholders::_1)},
            {2,  bind(&Telegram::findHistory, telegram, placeholders::_1)},
            {3,  bind(&Telegram::listFriends, telegram, placeholders::_1)},
            {4,  bind(&Telegram::addFriend, telegram, placeholders::_1)},
            {5,  bind(&Telegram::findRequest, telegram, placeholders::_1)},
            {6,  bind(&Telegram::delFriend, telegram, placeholders::_1)},
            {7,  bind(&Telegram::blockedLists, telegram, placeholders::_1)},
            {8,  bind(&Telegram::unblocked, telegram, placeholders::_1)},
            {9,  bind(&Telegram::group, telegram, placeholders::_1)},
            {10, bind(&Telegram::sendFile, telegram, placeholders::_1)},
            {11, bind(&Telegram::receiveFile, telegram, placeholders::_1)},
            {12, bind(&Telegram::viewProfile, telegram, placeholders::_1)}
    };
//    char *end_ptr;
//    int opt_int = (int) strtol(opt.c_str(), &end_ptr, 10);
//    if (*end_ptr != '\0' || opt.find(' ') != std::string::npos) {
//        std::cout << "选项格式错误 请重新输入" << std::endl;
//    }
    while (true) {
        operationMenu();
        string option;
        getline(cin, option);
        if (option.empty()) {
            cout << "输入为空" << endl;
            return;
        }
        if (option.length() > 4) {
            cout << "输入错误" << endl;
            continue;
        }
        if (option == "0") {

            sendMsg(fd, BACK);
            cout << "退出成功" << endl;
            break;
        }
        //下面这段逻辑在用户直接输入回车时，会提示用户重新输入，但cin>>option则没有提示
        char *end_ptr;
        //也可以使用atoi这个函数
        //int opt= atoi(option.c_str());
        int opt = (int) strtol(option.c_str(), &end_ptr, 10);
        if (opt == 0 || option.find(' ') != std::string::npos) {
            std::cout << "输入格式错误 请重新输入" << std::endl;
            continue;
        }

        if (FunctionOption.find(opt) == FunctionOption.end()) {
            //cout << "没有这个选项，请重新输入" << endl;
            continue;
        }
        //同步服务器
        sendMsg(fd, SYNC);
        //同步客户端
        telegram.sync(my_friends);
        FunctionOption[opt](my_friends);
    }
}

void operationMenu() {
    cout << "[1]开始聊天                  [2]历史记录" << endl;
    cout << "[3]查看好友                  [4]添加好友" << endl;
    cout << "[5]查看添加好友请求           [6]删除好友" << endl;
    cout << "[7]屏蔽好友                  [8]解除屏蔽" << endl;
    cout << "[9]群聊                      [10]发送文件" << endl;
    cout << "[11]接收文件                 [12]查看我的个人信息" << endl;
    cout << "按[0]返回" << endl;
    cout << "请输入您的选择" << endl;
}