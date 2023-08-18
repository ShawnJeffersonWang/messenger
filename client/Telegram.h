//
// Created by shawn on 23-8-2.
//

#ifndef CHATROOM_TELEGRAM_H
#define CHATROOM_TELEGRAM_H

#include <vector>
#include "User.h"

class Telegram {
public:
    Telegram(int fd, User user);

    void sync(std::vector<std::pair<string, User>> &my_friends);

    void startChat(std::vector<std::pair<string, User>> &my_friends);

    void findHistory(std::vector<std::pair<string, User>> &my_friends) const;

    void listFriends(std::vector<std::pair<string, User>> &);

    void addFriend(std::vector<std::pair<string, User>> &) const;

    void findRequest(std::vector<std::pair<string, User>> &my_friends) const;

    void delFriend(std::vector<std::pair<string, User>> &);

    void blockedLists(std::vector<std::pair<string, User>> &my_friends) const;

    void unblocked(std::vector<std::pair<string, User>> &my_friends) const;

    void group(std::vector<std::pair<string, User>> &my_friends) const;

    void sendFile(std::vector<std::pair<string, User>> &my_friends) const;

    void receiveFile(std::vector<std::pair<string, User>> &my_friends) const;

    void viewProfile(std::vector<std::pair<string, User>> &my_friends) const;

    static void groupMenu();

private:
    int fd;
    User user;
};

#endif //CHATROOM_TELEGRAM_H
