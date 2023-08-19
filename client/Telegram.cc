//
// Created by shawn on 23-8-7.
//

#include "Telegram.h"
#include "../utils/proto.h"
#include "../utils/IO.h"
#include "Notifications.h"
#include "GroupChat.h"
#include <thread>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <cstring>
#include <functional>
#include <map>
#include <csignal>

using namespace std;

//Telegram(int fd, const User &user) : fd(fd), user(user) {}
Telegram::Telegram(int fd, User user) : fd(fd), user(std::move(user)) {}

void Telegram::sync(vector<pair<string, User>> &my_friends) {
    //先清空my_friends存储的好友
    my_friends.clear();
    User _friend;
    string nums, friend_info;
    //接收服务器发送的好友数量
    recvMsg(fd, nums);
    int num = stoi(nums);
    for (int i = 0; i < num; i++) {
        //循环接收好友信息
        recvMsg(fd, friend_info);
        _friend.json_parse(friend_info);
        my_friends.emplace_back(user.getUID(), _friend);
    }
    system("clear");
    cout << "同步成功" << endl;
}

void Telegram::startChat(vector<pair<string, User>> &my_friends) {
    string temp;
    if (my_friends.empty()) {
        cout << "您当前没有好友捏... 请按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << "-------------------------------------" << endl;
    for (int i = 0; i < my_friends.size(); i++) {
        cout << i + 1 << ". " << my_friends[i].second.getUsername() << " " << my_friends[i].second.getUID() << endl;
    }
    cout << "-------------------------------------" << endl;
    int who;
    cout << "你想和谁聊天呀" << endl;
    while (!(cin >> who) || who < 0 || who > my_friends.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');
    system("clear");
    //发送"4"，服务器进入私聊业务
    sendMsg(fd, START_CHAT);
    //bug who必须要减1,不然只有一个好友的话，会导致数组索引越界
    who--;
    cout << "你好，我是" << my_friends[who].second.getUsername() << "快来与我聊天吧！" << endl;
    string records_index = user.getUID() + my_friends[who].second.getUID();
    //向服务器发送历史聊天记录索引
    sendMsg(fd, records_index);
    Message history;
    string nums;
    //接收历史消息数量
    recvMsg(fd, nums);
    int num = stoi(nums);
    //打印历史消息
    string history_message;
    for (int j = 0; j < num; j++) {
        //接收历史消息
        recvMsg(fd, history_message);
        history.json_parse(history_message);
        cout << "\t\t\t\t" << history.getTime() << endl;
        cout << history.getUsername() << "  :  " << history.getContent() << endl;
    }
    cout << YELLOW << "-------------------以上为历史消息-------------------" << RESET << endl;
    Message message(user.getUsername(), user.getUID(), my_friends[who].second.getUID());
    string friend_UID = my_friends[who].second.getUID();

    sendMsg(fd, friend_UID);
    string isBlocked;

    recvMsg(fd, isBlocked);
    if (isBlocked == "-1") {
        cout << EXCLAMATION << "! " << my_friends[who].second.getUsername()
             << "开启了朋友验证，你还不是他（她）朋友。请先发送朋友验证请求，对方验证通过后，才能聊天。" << RESET << endl;
        cout << "按任意键返回" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
    }
    //开线程接收私聊好友对方的消息
    thread work(chatReceived, fd, friend_UID);
    work.detach();
    string msg, json;
    while (true) {
        cin >> msg;
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        if (msg == "quit") {
            sendMsg(fd, EXIT);
            getchar();
            system("sync");
            break;
        }
        message.setContent(msg);
        json = message.to_json();
        //发送聊天消息
        //cout<<"json: "<<json<<endl;
        sendMsg(fd, json);
        //cout<<"ret: "<<ret<<endl;
    }
}

void Telegram::findHistory(vector<pair<string, User>> &my_friends) const {
    string temp;
    if (my_friends.empty()) {
        cout << "您当前没有好友捏... 请按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    sendMsg(fd, HISTORY);
    cout << "好友列表" << endl;
    cout << "-----------------------------------" << endl;
    for (int i = 0; i < my_friends.size(); ++i) {
        cout << i + 1 << ":" << my_friends[i].second.getUsername() << endl;
    }
    cout << "-----------------------------------" << endl;
    int who;
    while (!(cin >> who) || who < 0 || who > my_friends.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');
    //发送想要查看的历史记录的好友的UID
    who--;
    sendMsg(fd, my_friends[who].second.getUID());
    string nums;
    //接收服务器发来的历史记录数量
    recvMsg(fd, nums);
    int num = stoi(nums);
    if (num == 0) {
        cout << "您还没有与他聊天" << endl;
    } else {
        Message message;
        string history_message;
        for (int i = 0; i < num; i++) {
            //接收服务器发送的历史消息记录
            recvMsg(fd, history_message);
            message.json_parse(history_message);
            cout << "\t\t\t\t" << message.getTime() << endl;
            cout << message.getUsername() << ": " << message.getContent() << endl;
        }
    }
    cout << "请按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void Telegram::listFriends(vector<pair<string, User>> &my_friends) {
    string temp;
    if (my_friends.empty()) {
        cout << "您当前没有好友捏... 请按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    sendMsg(fd, LIST_FRIENDS);
    string is_online;
    cout << user.getUsername() << "的好友列表" << endl;
    cout << "-----------------------------------------" << endl;
    //发送好友数量
    sendMsg(fd, to_string(my_friends.size()));
    for (int i = 0; i < my_friends.size(); ++i) {
        //循环发送好友的UID来查询信息
        sendMsg(fd, my_friends[i].second.getUID());
        //接收好友是否在线的信息
        recvMsg(fd, is_online);
        if (is_online == "1") {
            cout << GREEN << i + 1 << ". " << my_friends[i].second.getUsername() << RESET << endl;
            //bug 没有加else 导致一个人输出了两遍
        } else {
            cout << i + 1 << ". " << my_friends[i].second.getUsername() << endl;
        }
    }
    cout << "-----------------------------------------" << endl;
    cout << "按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void Telegram::addFriend(vector<pair<string, User>> &) const {
    sendMsg(fd, ADD_FRIEND);
    string UID;
    while (true) {
        cout << "请输入你要添加好友的UID" << endl;
        getline(cin, UID);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        if (UID.find(' ') != string::npos) {
            cout << "帐号录入不允许出现空格" << endl;
            continue;
        }

        sendMsg(fd, UID);
        string temp;
        //接收服务器发来的信号判断是否可以添加
        recvMsg(fd, temp);
        if (temp == "-1") {
            cout << "该用户不存在" << endl;
            return;
        } else if (temp == "-2") {
            cout << "该用户已经是你的好友，无法多次添加" << endl;
            return;
        } else if (temp == "-3") {
            cout << "你不能添加自己为好友！" << endl;
            return;
        }
        break;
    }
    User her;
    string user_info;
    //接收服务器从数据库查找的用户信息
    recvMsg(fd, user_info);
    her.json_parse(user_info);
    cout << "您已成功发出好友申请，等待" << her.getUsername() << "的同意" << endl;
}

void Telegram::findRequest(vector<pair<string, User>> &my_friends) const {
    sendMsg(fd, FIND_REQUEST);
    string nums;

    recvMsg(fd, nums);
    int num = stoi(nums);
    if (num == 0) {
        string temp;
        cout << "目前没有好友申请，按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << "你收到" << num << "条好友申请" << endl;
    string friendRequestName;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, friendRequestName);
        cout << "收到" << friendRequestName << "的好友申请 [1]同意 [0]拒绝" << endl;
        int choice;
        while (!(cin >> choice) || (choice != 0 && choice != 1)) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        string reply;
        if (choice == 0) {
            reply = "REFUSED";
        } else {
            reply = "ACCEPTED";
        }

        sendMsg(fd, reply);
        string request_info;
        if (choice == 1) {
            cout << "好友添加成功" << endl;
            User newFriend;
            recvMsg(fd, request_info);
            newFriend.json_parse(request_info);
            my_friends.emplace_back(request_info, newFriend);
        } else {
            cout << "您拒绝了" << friendRequestName << "的请求" << endl;
        }
    }
}

void Telegram::delFriend(vector<pair<string, User>> &my_friends) {
    string temp;
    if (my_friends.empty()) {
        cout << "您当前没有好友捏... 请按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << "     " << user.getUsername() << "的好友列表" << endl;
    cout << "----------------------------------------" << endl;
    for (int i = 0; i < my_friends.size(); ++i) {
        cout << i + 1 << "." << my_friends[i].second.getUsername() << endl;
    }
    cout << "----------------------------------------" << endl;
    while (true) {
        cout << "请输入要删除的好友" << endl;
        string del;
        getline(cin, del);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        int who;
        while (!(cin >> who) || who < 0 || who > my_friends.size()) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        //向服务器发送删除好友的信号
        sendMsg(fd, DEL_FRIEND);
        //发送删除好友的UID
        who--;

        sendMsg(fd, my_friends[who].second.getUID());
    }
}

void Telegram::blockedLists(vector<pair<string, User>> &my_friends) const {
    string temp;
    if (my_friends.empty()) {
        cout << "您当前没有好友捏... 请按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << "好友列表" << endl;
    cout << "-----------------------------------------" << endl;
    for (int i = 0; i < my_friends.size(); ++i) {
        cout << i + 1 << ": " << my_friends[i].second.getUsername() << endl;
    }
    cout << "-----------------------------------------" << endl;
    cout << "请输入你要屏蔽的好友的序号" << endl;
    int who;
    while (!(cin >> who) || who < 0 || who > my_friends.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');

    sendMsg(fd, BLOCKED_LISTS);
    who--;

    sendMsg(fd, my_friends[who].second.getUID());
    cout << "您已成功屏蔽" << my_friends[who].second.getUsername() << ", 按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void Telegram::unblocked(vector<pair<string, User>> &my_friends) const {
    sendMsg(fd, UNBLOCKED);
    cout << "已屏蔽好友" << endl;
    string nums;
    //接收服务器发送的屏蔽名单数量
    recvMsg(fd, nums);
    int num = stoi(nums);
    if (num == 0) {
        string temp;
        cout << "您的屏蔽列表为空" << endl;
        cout << "请按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    string blocked_info;
    User blocked_user;
    vector<User> blocked_users;
    for (int i = 0; i < num; i++) {
        //循环接收屏蔽名单信息
        recvMsg(fd, blocked_info);
        blocked_user.json_parse(blocked_info);
        blocked_users.push_back(blocked_user);
        cout << i + 1 << ". " << blocked_user.getUsername() << endl;
    }
    cout << "请输入你要解除屏蔽的好友序号" << endl;
    int who;
    while (!(cin >> who) || who < 0 || who > num) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');
    //向服务器发送解除屏蔽的UID
    who--;

    sendMsg(fd, blocked_users[who].getUID());
    cout << "您已经成功解除了对" << blocked_users[who].getUsername() << "的屏蔽，请按任意键退出" << endl;
    getline(cin, blocked_info);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void Telegram::group(vector<pair<string, User>> &my_friends) const {
    sendMsg(fd, GROUP);
    vector<Group> joinedGroup;
    vector<Group> managedGroup;
    vector<Group> createdGroup;
    GroupChat groupChat(fd, user);
    groupChat.sync(createdGroup, managedGroup, joinedGroup);
    int option;
    while (true) {
        groupMenu();
        while (!(cin >> option)) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');

        sendMsg(fd, "11");
        groupChat.sync(createdGroup, managedGroup, joinedGroup);
        if (option == 0) {
            sendMsg(fd, BACK);
            return;
        }
        if (option == 1) {
            groupChat.startChat(joinedGroup);
            continue;
        } else if (option == 2) {
            groupChat.createGroup();
            continue;
        } else if (option == 3) {
            groupChat.joinGroup();
            continue;
        } else if (option == 4) {
            groupChat.groupHistory(joinedGroup);
            continue;
        } else if (option == 5) {
            groupChat.managedGroup(managedGroup);
            continue;
        } else if (option == 6) {
            groupChat.managedCreatedGroup(createdGroup);
            continue;
        } else if (option == 7) {
            groupChat.showMembers(joinedGroup);
            continue;
        } else if (option == 8) {
            groupChat.quit(joinedGroup);
            continue;
        } else if (option == 9) {
            groupChat.showJoinedGroup(joinedGroup);
            continue;
        } else if (option == 10) {
            groupChat.showManagedGroup(managedGroup);
            continue;
        } else if (option == 11) {
            groupChat.showCreatedGroup(createdGroup);
            continue;
        }
    }

}

void Telegram::sendFile(vector<pair<string, User>> &my_friends) const {
    system("clear");
    if (my_friends.empty()) {
        cout << "您当前还没有好友" << endl;
        cout << "按任意键退出" << endl;
        string temp;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }

    sendMsg(fd, SEND_FILE);
    cout << "-------------------------------------------------" << endl;
    for (int i = 0; i < my_friends.size(); ++i) {
        cout << i + 1 << ". " << my_friends[i].second.getUsername() << endl;
    }
    cout << "-------------------------------------------------" << endl;
    cout << "请选择你要发送文件的好友,按[0]返回" << endl;
    int who;
    while (!(cin >> who) || who < 0 || who > my_friends.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');
    if (who == 0) {

        sendMsg(fd, BACK);
        return;
    }
    who--;

    sendMsg(fd, my_friends[who].second.to_json());
    string filePath;
    int inputFile;
    off_t offset = 0;
    struct stat fileStat;
    while (true) {
        cout << "请输入文件路径" << endl;
        cin >> filePath;
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        inputFile = open(filePath.c_str(), O_RDONLY);
        if (inputFile == -1) {
            cerr << "无法打开文件" << endl;
            continue;
        }
        offset = 0;
        if (fstat(inputFile, &fileStat) == -1) {
            cerr << "获取文件状态失败" << endl;
            close(inputFile);
            continue;
        }

        sendMsg(fd, filePath);
        filesystem::path path(filePath);
        string fileName = path.filename().string();

        sendMsg(fd, fileName);
        break;
    }
    off_t fileSize = fileStat.st_size;

    sendMsg(fd, to_string(fileSize));
    cout << "开始发送文件" << endl;
    ssize_t bytesSent = sendfile(fd, inputFile, &offset, fileSize);
    system("sync");
    if (bytesSent == -1) {
        cerr << "传输文件失败" << endl;
        close(inputFile);
    }
    cout << "成功传输" << bytesSent << "字节的数据" << endl;
    close(inputFile);
    cout << "按任意键返回" << endl;
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
    system("clear");
}

void Telegram::receiveFile(vector<std::pair<string, User>> &my_friends) const {
    sendMsg(fd, RECEIVE_FILE);
    string nums;
    Message message;
    string filePath;
    string fileName;
    //先接收服务器发来的文件数
    recvMsg(fd, nums);
    int num = stoi(nums);
    cout << "您有" << num << "个文件待接收" << endl;
    string file_info;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, file_info);
        message.json_parse(file_info);
        cout << "你收到" << message.getUsername() << "的文件" << message.getGroupName() << endl;
        cout << "是否要接收[1]YES, [0]NO" << endl;
        int choice;
        while (!(cin >> choice) || (choice != 0 && choice != 1)) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        string reply;
        if (choice == 1) {
            reply = "YES";
        } else {
            reply = "NO";
        }

        sendMsg(fd, reply);
        if (choice == 0) {
            string temp;
            cout << "您拒绝接收了该文件, 按任意键返回" << endl;
            getline(cin, temp);
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            return;
        }
        fileName = message.getGroupName();
        filePath = "./fileBuffer/" + fileName;
        if (!filesystem::exists("./fileBuffer")) {
            filesystem::create_directories("./fileBuffer");
        }
        ofstream ofs(filePath);
        string ssize;
        char buf[BUFSIZ];
        int n;

        recvMsg(fd, ssize);
        off_t size = stoll(ssize);
        while (size > 0) {
            if (size > sizeof(buf)) {
                n = read_n(fd, buf, sizeof(buf));
            } else {
                n = read_n(fd, buf, size);
            }
            //cout << "size = " << size << endl;
            size -= n;
            ofs.write(buf, n);
        }
        cout << "文件接收完成" << endl;
        ofs.close();
    }
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "按任意键返回" << endl;
        return;
    }
    system("clear");
}

//bug UI提示错误
void Telegram::groupMenu() {
    cout << "[1]开始聊天                     [2]创建群聊" << endl;
    cout << "[3]加入群聊                     [4]查看群聊历史记录" << endl;
    cout << "[5]管理我的群                   [6]管理我创建的群" << endl;
    cout << "[7]查看群成员                   [8]退出群聊" << endl;
    cout << "[9]查看我加入的群                [10]查看我管理的群" << endl;
    cout << "[11]查看我创建的群" << endl;
    cout << "[0]返回" << endl;
    cout << "请输入您的选择" << endl;
}

void Telegram::viewProfile(vector<std::pair<string, User>> &my_friends) const {
    cout << "您的用户创建时间为: " << user.getMyTime() << endl;
    cout << "您的UID为: " << user.getUID() << endl;
    cout << "您的用户名是: " << user.getUsername() << endl;
    cout << "按任意键退出" << endl;
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}


