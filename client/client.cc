#include <functional>
#include <iostream>
#include <map>
#include <cstdint>
#include <csignal>
#include "client.h"
#include "../utils/TCP.h"
#include "StartMenu.h"
#include "../utils/proto.h"
#include "OperationMenu.h"

using namespace std;

//void signalHandler(int signum) {
//    cout << "Ctrl+C被屏蔽了!" << endl;
//}

//测试帐号
//直接将void start_menu(int fd, User &user)写在main函数中，减少user和fd的一次转发，并且可以直接用上循环的逻辑
int main(int argc, char *argv[]) {
    if (argc == 1) {
        IP = "10.30.0.202";
        PORT = 8888;
    } else if (argc == 3) {
        IP = argv[1];
        PORT = stoi(argv[2]);
    } else {
        // 错误情况
        cerr << "Invalid number of arguments. Usage: program_name [IP] [port]" << endl;
        return 1;
    }
    //signal(SIGINT, signalHandler);
    int fd;
    User user;
    fd = Socket();
    Connect(fd, IP, PORT);
    //User user;
    //int option;
    // 用户输入选项和函数映射关系
//    map<int, function<void(int)>> LoginOption = {
//            {1, login},
//            {2, client_register},
//    };
    while (true) {
        start_UI();
        string option;
        getline(cin, option);
        if (option.empty()) {
            cout << "输入为空！" << endl;
            continue;
        }
        if (option != "0" && option != "1" && option != "2") {
            cout << "没有这个选项！" << endl;
            continue;
        }
        if (option == "0") {
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
//        while (!(cin >> option) || option < 0 || option > 4) {
//            if (cin.eof()) {
//                cout << "读到文件结尾" << endl;
//                return -1;
//            }
//            cout << "输入格式错误 请重新输入" << endl;
//            cin.clear();
//            cin.ignore(INT32_MAX, '\n');
//        }
//        //bug:如果此时不ignore的话，缓冲区还有数据，账号输入环节会直接跳过
//        cin.ignore(INT32_MAX, '\n');

        //system("clear");
        // 判断输入的命令是否存在
//        if (LoginOption.find(option) == LoginOption.end()) {
//            std::cout << "没有这个选项 请重新输入" << std::endl;
//            continue;
//        }
//        //bug:之前没有在上方的if判断里加上continue导致没有这个选项，仍然调用了函数
//        LoginOption[option](fd);
        if (opt == 1) {
            if (login(fd, user)) {
                clientOperation(fd, user);
            }
            continue;
        }
        if (opt == 2) {
            client_register(fd);
        }
    }
}

void start_UI() {
    cout << "[1]登录               [2]注册" << endl;
    cout << "[0]退出" << endl;
    cout << "请输入你的选择" << endl;
}