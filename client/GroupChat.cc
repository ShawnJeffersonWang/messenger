//
// Created by shawn on 23-8-10.
//

#include "GroupChat.h"
#include "User.h"
#include "IO.h"
#include "Group.h"
#include "proto.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <thread>
#include <functional>
#include <map>
#include "Notifications.h"

using namespace std;

GroupChat::GroupChat(int fd, const User &user) : fd(fd), user(user) {
    joined = "joined" + user.getUID();
    created = "created" + user.getUID();
    managed = "managed" + user.getUID();
}

void GroupChat::sync(vector<Group> &createdGroup, vector<Group> &managedGroup, vector<Group> &joinedGroup) const {
    createdGroup.clear();
    managedGroup.clear();
    joinedGroup.clear();
    string nums;

    recvMsg(fd, nums);
    int num = stoi(nums);
    string group_info;
    for (int i = 0; i < num; i++) {
        Group group;

        recvMsg(fd, group_info);
        group.json_parse(group_info);
        createdGroup.push_back(group);
    }

    recvMsg(fd, nums);
    num = stoi(nums);
    for (int i = 0; i < num; i++) {
        Group group;

        recvMsg(fd, group_info);
        group.json_parse(group_info);
        managedGroup.push_back(group);
    }

    recvMsg(fd, nums);
    num = stoi(nums);
    for (int i = 0; i < num; i++) {
        Group group;

        recvMsg(fd, group_info);
        group.json_parse(group_info);
        joinedGroup.push_back(group);
    }
}

void GroupChat::startChat(vector<Group> &joinedGroup) {
    string temp;
    if (joinedGroup.empty()) {
        cout << "您当前没有加入任何群聊" << endl;
        cout << "按任意键返回" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            return;
        }
        return;
    }
    cout << user.getUsername() << "加入的群聊" << endl;
    cout << "---------------------------------------" << endl;
    for (int i = 0; i < joinedGroup.size(); i++) {
        cout << i + 1 << ". " << joinedGroup[i].getGroupName() << endl;
    }
    cout << "---------------------------------------" << endl;
    int which;
    while (!(cin >> which) || which < 0 || which > joinedGroup.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');

    sendMsg(fd, "1");
    which--;

    sendMsg(fd, joinedGroup[which].to_json());
    cout << joinedGroup[which].getGroupName() << endl;
    string nums;

    int ret = recvMsg(fd, nums);
    if (ret == 0) {
        return;
    }
    int num = stoi(nums);
    string msg;
    Message history_message;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, msg);
        history_message.json_parse(msg);
        cout << "\t\t\t\t" << history_message.getTime() << endl;
        cout << history_message.getUsername() << ": " << history_message.getContent() << endl;
    }
    cout << YELLOW << "-----------------------以上为历史消息-----------------------" << RESET << endl;
    Message message(user.getUsername(), user.getUID(), joinedGroup[which].getGroupUid());
    message.setGroupName(joinedGroup[which].getGroupName());
    //开线程接收群成员消息
    thread work(chatReceived, fd, joinedGroup[which].getGroupUid());
    work.detach();
    string m;
    while (true) {
        cin >> m;
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        if (m == "quit") {
            sendMsg(fd, EXIT);
            getchar();
            break;
        }
        message.setContent(m);
        string json_msg;
        json_msg = message.to_json();

        sendMsg(fd, json_msg);
    }
}

void GroupChat::createGroup() {
    sendMsg(fd, "2");
    string groupName;
    while (true) {
        cout << "您要创建的群聊名称是什么" << endl;
        getline(cin, groupName);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        if (groupName.find(' ') != string::npos) {
            cout << "群聊名称不能出现空格" << endl;
            continue;
        }
        break;
    }
    Group group(groupName, user.getUID());

    sendMsg(fd, group.to_json());
    cout << "创建成功，该群的群号为：" << group.getGroupUid() << endl;
    string temp;
    cout << "按任意键返回" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::joinGroup() const {
    sendMsg(fd, "3");
    string group_uid;
    while (true) {
        cout << "输入你要加入的群聊UID" << endl;
        getline(cin, group_uid);
        if (cin.eof()) {
            cout << "文件读到结尾" << endl;
            return;
        }
        if (group_uid.find(' ') != string::npos) {
            cout << "群聊UID没有空格" << endl;
            continue;
        }
        break;
    }

    sendMsg(fd, group_uid);
    string response;

    recvMsg(fd, response);
    if (response == "-1") {
        cout << "该群不存在" << endl;
    } else if (response == "-2") {
        cout << "您已经是该群成员" << endl;
    }
    cout << "请求已经发出，等待同意" << endl;
    string temp;
    cout << "按任意键返回" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::groupHistory(const std::vector<Group> &joinedGroup) {
    string temp;
    if (joinedGroup.empty()) {
        cout << "当前没有加入的群... 按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << user.getUsername() << "加入的群聊" << endl;
    cout << "----------------------------------" << endl;
    for (int i = 0; i < joinedGroup.size(); i++) {
        cout << i + 1 << ". " << joinedGroup[i].getGroupName() << endl;
    }
    cout << "----------------------------------" << endl;
    cout << "输入要查看的群" << endl;
    int which;
    while (!(cin >> which) || which < 0 || which > joinedGroup.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');

    sendMsg(fd, "4");
    which--;

    sendMsg(fd, joinedGroup[which].getGroupUid());
    Message message;
    string nums;

    recvMsg(fd, nums);
    int num = stoi(nums);
    if (num == 0) {
        cout << "该群暂未有人发言" << endl;
        return;
    }
    string history_msg;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, history_msg);
        message.json_parse(history_msg);
        cout << message.getTime() << message.getUsername() << ": " << message.getContent() << endl;
    }
    cout << "按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::managedMenu() {
    cout << "[1]处理入群申请" << endl;
    cout << "[2]处理群用户" << endl;
    cout << "[0]返回" << endl;
}

void GroupChat::managedGroup(vector<Group> &managedGroup) const {
    string temp;
    if (managedGroup.empty()) {
        cout << "您当前还没有可以管理的群... 按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << "-----------------------------------" << endl;
    for (int i = 0; i < managedGroup.size(); i++) {
        cout << i + 1 << managedGroup[i].getGroupName() << endl;
    }
    cout << "-----------------------------------" << endl;
    cout << "选择你要管理的群" << endl;
    int which;
    while (!(cin >> which) || which < 0 || which > managedGroup.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');
    sendMsg(fd, "5");
    which--;

    sendMsg(fd, managedGroup[which].to_json());
    while (true) {
        managedMenu();
        int choice;
        while (!(cin >> choice) || (choice != 1 && choice != 2 && choice != 0)) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        if (choice == 0) {

            sendMsg(fd, BACK);
            break;
        }
        if (choice == 1) {
            approve();
            break;
        } else if (choice == 2) {
            remove(managedGroup[which]);
            break;
        }

    }
}

void GroupChat::approve() const {
    sendMsg(fd, "1");
    string nums;

    recvMsg(fd, nums);
    int num = stoi(nums);
    if (num == 0) {
        cout << "暂无入群申请" << endl;
        return;
    }
    string buf;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, buf);
        cout << "收到" << buf << "的入群申请" << endl;
        cout << "[y]YES,[n]NO" << endl;
        string choice;
        getline(cin, choice);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        if (choice != "y" && choice != "n") {
            cout << "输入格式错误" << endl;
            continue;
        }

        sendMsg(fd, choice);
        if (choice == "y") {
            cout << "添加成功" << endl;
        } else {
            cout << "添加失败" << endl;
        }
    }
    cout << "按任意键返回" << endl;
    getline(cin, buf);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::remove(Group &group) const {
    sendMsg(fd, "2");
    string buf;
    User member;
    vector<User> arr;
    //接收服务器发送的群员数量
    recvMsg(fd, buf);
    int num = stoi(buf);
    for (int i = 0; i < num; i++) {
        //接收服务器发送的群员信息
        recvMsg(fd, buf);
        member.json_parse(buf);
        arr.push_back(member);
        if (member.getUID() == group.getOwnerUid()) {
            cout << i + 1 << "." << member.getUsername() << "(群主)" << endl;
        } else {
            cout << i + 1 << "." << member.getUsername() << endl;
        }
    }
    while (true) {
        cout << "你要踢谁,按[0]返回" << endl;
        int who;
        while (!(cin >> who) || who < 0 || who > arr.size()) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        if (who == 0) {
            sendMsg(fd, "0");
            return;
        }
        who--;
        if (arr[who].getUID() == group.getOwnerUid()) {
            cout << "该用户是群主，你不能踢！" << endl;
            continue;
        }

        sendMsg(fd, arr[who].to_json());
        cout << "删除成功，按任意键返回" << endl;
        getline(cin, buf);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
    }
}

void GroupChat::ownerMenu() {
    cout << "[1]设置管理员" << endl;
    cout << "[2]撤销管理员" << endl;
    cout << "[3]解散群聊" << endl;
    cout << "[0]返回" << endl;
}

void GroupChat::managedCreatedGroup(vector<Group> &createdGroup) {
    string temp;
    if (createdGroup.empty()) {
        cout << "您当前还没有创建群... 按任意键退出" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << user.getUsername() << "的群" << endl;
    cout << "------------------------------" << endl;
    for (int i = 0; i < createdGroup.size(); i++) {
        cout << i + 1 << ". " << createdGroup[i].getGroupName() << endl;
    }
    cout << "------------------------------" << endl;
    cout << "您要整哪个群" << endl;
    int which;
    while (!(cin >> which) || which < 0 || which > createdGroup.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');

    sendMsg(fd, "6");
    which--;

    sendMsg(fd, createdGroup[which].to_json());
    GroupChat gc;
    map<int, function<void(Group &)>> ownerOperation = {
            {1, bind(&GroupChat::appointAdmin, &gc, placeholders::_1)},
            {2, bind(&GroupChat::revokeAdmin, &gc, placeholders::_1)},
            {3, bind(&GroupChat::deleteGroup, &gc, placeholders::_1)},
    };
    while (true) {
        ownerMenu();
        int choice;
        while (!(cin >> choice) || choice < 0 || choice > 3) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        if (choice == 0) {
            sendMsg(fd, "0");
            return;
        }
        ownerOperation[choice](createdGroup[which]);
    }

}

void GroupChat::appointAdmin(Group &createdGroup) const {
    sendMsg(fd, "1");
    vector<User> arr;
    string nums;
    User member;

    recvMsg(fd, nums);
    int num = stoi(nums);
    string member_info;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, member_info);
        member.json_parse(member_info);
        arr.push_back(member);
        if (member.getUID() == createdGroup.getOwnerUid()) {
            cout << i + 1 << ". " << member.getUsername() << "群主" << endl;
        } else {
            cout << i + 1 << ". " << member.getUsername() << endl;
        }
    }
    int who;
    while (true) {
        cout << "您要任命谁为管理员" << endl;
        while (!(cin >> who) || who < 0 || who > arr.size()) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        who--;
        if (arr[who].getUID() == createdGroup.getOwnerUid()) {
            cout << "该用户为群主" << endl;
            continue;
        }
        break;
    }

    sendMsg(fd, arr[who].to_json());
    string reply;

    recvMsg(fd, reply);
    if (reply == "-1") {
        cout << "该用户为管理员，无法多次设置" << endl;
        return;
    }
    cout << "按任意键退出" << endl;
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::revokeAdmin(Group &createdGroup) const {
    sendMsg(fd, "2");
    string nums;

    recvMsg(fd, nums);
    int num = stoi(nums);
    string admin_info;
    User admin;
    vector<User> arr;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, admin_info);
        admin.json_parse(admin_info);
        arr.push_back(admin);
        if (admin.getUID() == createdGroup.getOwnerUid()) {
            cout << i + 1 << admin.getUsername() << "群主" << endl;
        } else {
            cout << i + 1 << admin.getUsername() << endl;
        }
    }
    int who;
    while (true) {
        cout << "选择你要取消的人" << endl;
        while (!(cin >> who) || who < 0 || who > arr.size()) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        who--;
        if (arr[who].getUID() == createdGroup.getOwnerUid()) {
            cout << "不能取消群主的管理权限" << endl;
            continue;
        }
        break;
    }

    sendMsg(fd, arr[who].to_json());
    cout << "撤销成功，按任意键返回" << endl;
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::deleteGroup(Group &createdGroup) const {
    sendMsg(fd, "3");
    cout << "解散成功，按任意键返回" << endl;
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::showMembers(std::vector<Group> &group) {
    cout << user.getUsername() << endl;
    cout << "--------------------------" << endl;
    for (int i = 0; i < group.size(); i++) {
        cout << i + 1 << ". " << group[i].getGroupName() << endl;
    }
    cout << "--------------------------" << endl;
    cout << "你要查看哪个群" << endl;
    int which;
    while (!(cin >> which) || which < 0 || which > group.size()) {
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        cout << "输入格式错误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n');

    sendMsg(fd, "7");
    which--;
    cout << group[which].getGroupName() << endl;

    sendMsg(fd, group[which].to_json());
    string nums;

    recvMsg(fd, nums);
    int num = stoi(nums);
    string member_info;
    for (int i = 0; i < num; i++) {

        recvMsg(fd, member_info);
        cout << i + 1 << ". " << member_info << endl;
    }
    cout << "按任意键返回" << endl;
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::quit(vector<Group> &joinedGroup) {
    string temp;
    if (joinedGroup.empty()) {
        cout << "您当前没有加入任何群聊" << endl;
        cout << "按任意键返回" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            return;
        }
        return;
    }
    cout << user.getUsername() << endl;
    cout << "-------------------------------------------" << endl;
    for (int i = 0; i < joinedGroup.size(); i++) {
        if (joinedGroup[i].getOwnerUid() == user.getUID()) {
            cout << i + 1 << ". " << joinedGroup[i].getGroupName() << "(您是群主)" << endl;
        } else {
            cout << i + 1 << ". " << joinedGroup[i].getGroupName() << endl;
        }
    }
    cout << "-------------------------------------------" << endl;
    int which;
    while (true) {
        cout << "输入你要退出的群" << endl;
        while (!(cin >> which) || which < 0 || which > joinedGroup.size()) {
            if (cin.eof()) {
                cout << "读到文件结尾" << endl;
                return;
            }
            cout << "输入格式错误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n');
        which--;
        if (joinedGroup[which].getOwnerUid() == user.getUID()) {
            cout << "您是该群群主，不能退出" << endl;
            continue;
        }
        break;
    }
    //退群是"8"
    sendMsg(fd, "8");

    sendMsg(fd, joinedGroup[which].to_json());
    cout << "您已退出该群，按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}


void GroupChat::showJoinedGroup(const std::vector<Group> &joinedGroup) {
    if (joinedGroup.empty()) {
        cout << "您未加入任何群聊" << endl;
        cout << "按任意键返回" << endl;
        string temp;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << user.getUsername() << "加入的群聊" << endl;
    cout << "------------------------------------" << endl;
    for (int i = 0; i < joinedGroup.size(); ++i) {
        cout << i + 1 << ". " << joinedGroup[i].getGroupName() << "(" << joinedGroup[i].getGroupUid() << ")" << endl;
    }
    cout << "------------------------------------" << endl;
    cout << "按任意键返回" << endl;
    string temp;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::showManagedGroup(vector<Group> &managedGroup) {
    string temp;
    if (managedGroup.empty()) {
        cout << "您未管理任何群" << endl;
        cout << "按任意键返回" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << user.getUsername() << "管理的群" << endl;
    cout << "----------------------------------" << endl;
    for (int i = 0; i < managedGroup.size(); ++i) {
        cout << i + 1 << ". " << managedGroup[i].getGroupName() << "(" << managedGroup[i].getGroupUid() << ")" << endl;
    }
    cout << "----------------------------------" << endl;
    cout << "按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}

void GroupChat::showCreatedGroup(std::vector<Group> &createdGroup) {
    string temp;
    if (createdGroup.empty()) {
        cout << "您未创建任何群" << endl;
        cout << "按任意键返回" << endl;
        getline(cin, temp);
        if (cin.eof()) {
            cout << "读到文件结尾" << endl;
            return;
        }
        return;
    }
    cout << user.getUsername() << "创建的群" << endl;
    cout << "----------------------------------" << endl;
    for (int i = 0; i < createdGroup.size(); ++i) {
        cout << i + 1 << ". " << createdGroup[i].getGroupName() << "(" << createdGroup[i].getGroupUid() << ")" << endl;
    }
    cout << "----------------------------------" << endl;
    cout << "按任意键退出" << endl;
    getline(cin, temp);
    if (cin.eof()) {
        cout << "读到文件结尾" << endl;
        return;
    }
}
