//
// Created by shawn on 23-8-1.
//

#ifndef CHATROOM_TRANSACTION_H
#define CHATROOM_TRANSACTION_H

#include "../utils/User.h"

void synchronize(int fd, User &user);

void start_chat(int fd, User &user);

void history(int fd, User &user);

void list_friend(int fd, User &user);

void add_friend(int fd, User &user);

void findRequest(int fd, User &user);

void del_friend(int fd, User &user);

void blockedLists(int fd, User &user);

void unblocked(int fd, User &user);

void group(int fd, User &user);

void send_file(int fd, User &user);

void receive_file(int fd, User &user);


#endif //CHATROOM_TRANSACTION_H
