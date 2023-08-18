cmake_minimum_required(VERSION 3.22)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 17)
#加上这个可以直接#include "User.h"而不用#include "../utils/User.h"
include_directories(
        server
        client
        utils
)

add_executable(
        server.out
        server/server.cc
        server/LoginHandler.cc
        server/Transaction.cc
        server/Redis.cc
        server/ThreadPool.cc
        server/group_chat.cc
        utils/User.cc
        utils/TCP.cc
        utils/IO.cc
        utils/proto.cc
        utils/Group.cc
)

add_executable(
        client.out
        client/client.cc
        client/StartMenu.cc
        client/OperationMenu.cc
        client/Notifications.cc
        client/Telegram.cc
        client/GroupChat.cc
        utils/User.cc
        utils/TCP.cc
        utils/IO.cc
        utils/proto.cc
        utils/Group.cc
)

add_executable(
        test.out
        utils/User.cc
        utils/Group.cc
        server/test.cc
)

target_link_libraries(
        server.out
        jsoncpp
        hiredis
)

target_link_libraries(
        client.out
        jsoncpp
)

target_link_libraries(
        test.out
        jsoncpp
)