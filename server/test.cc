//
// Created by shawn on 23-8-1.
//
#include <iostream>
#include "User.h"
#include "Group.h"

using namespace std;

#include <cstdio>
#include <termios.h>
#include <unistd.h>

//int getch();
//
//void get_password(char *password);
//
//int main() {
//    char password[20];
//    get_password(password);
//    printf("%s\n", password);
//
//    return 0;
//}
//
//int getch() {
//    int ch;
//    struct termios tm, tm_old;
//    tcgetattr(STDIN_FILENO, &tm);
//    tm_old = tm;
//    tm.c_lflag &= ~(ICANON | ECHO);
//    tcsetattr(STDIN_FILENO, TCSANOW, &tm);
//    ch = getchar();
//    tcsetattr(STDIN_FILENO, TCSANOW, &tm_old);
//    return ch;
//}
//
//void get_password(char *password) {
//    int i = 0;
//    char ch;
//
//    printf("Enter password: ");
//    while ((ch = getch()) != '\n') {
//        if (ch == '\b') {
//            if (i > 0) {
//                printf("\b \b");
//                i--;
//            }
//        } else {
//            password[i] = ch;
//            printf("*");
//            i++;
//        }
//    }
//    password[i] = '\0';
//    printf("\n");
//}
//编译时没有加上User.cc导致链接错误
int main() {
    //User user;
    //string time=user.getUID();
    cout << (int) NULL << endl;
    Group group("1", "2");
    cout << "UID: " << group.getGroupUid() << endl;
}