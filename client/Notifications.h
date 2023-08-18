//
// Created by shawn on 23-8-2.
//

#ifndef CHATROOM_NOTIFICATIONS_H
#define CHATROOM_NOTIFICATIONS_H

#include "User.h"

void announce(string UID);

void chatReceived(int fd, string UID);

bool isNumericString(const std::string &str);

#endif //CHATROOM_NOTIFICATIONS_H
