//
// Created by shawn on 23-8-13.
//

#ifndef CHATROOM_GROUP_CHAT_H
#define CHATROOM_GROUP_CHAT_H

#include <utility>

#include "User.h"
#include "Group.h"

class GroupChat {
public:
    GroupChat() = default;

    GroupChat(int fd, const User &user);

    void sync();

    void startChat();

    void createGroup();

    void joinGroup();

    void groupHistory() const;

    void managedGroup() const;

    void approve(Group &group) const;

    void remove(Group &group) const;

    void managedCreatedGroup() const;

    void appointAdmin(Group &createdGroup) const;

    void revokeAdmin(Group &createdGroup) const;

    static void deleteGroup(Group &createdGroup);

    void showMembers() const;

    void quit();

private:
    int fd;
    User user;
    string joined;
    string managed;
    string created;
};

#endif //CHATROOM_GROUP_CHAT_H
