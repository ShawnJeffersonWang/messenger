//
// Created by shawn on 23-7-27.
//
#ifndef CHATROOM_LOGINHANDLER_H
#define CHATROOM_LOGINHANDLER_H

#include "User.h"

void serverLogin(int epfd, int fd);

void findPassword(int fd, const string &UID);

void serverRegister(int epfd, int fd);

void serverOperation(int fd, User &user);

void notify(int fd);

#endif  // CHATROOM_LOGINHANDLER_H
