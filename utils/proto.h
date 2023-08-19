//
// Created by shawn on 23-8-7.
//
#ifndef CHATROOM_PROTO_H
#define CHATROOM_PROTO_H

#include <string>

using std::string;
//maybe 消息不能重复
//type
const string LOGIN = "1";
const string REGISTER = "2";
const string NOTIFY = "3";
//客户端子线程进行实时通知，私聊，群聊
//transaction
const string START_CHAT = "4";
const string HISTORY = "5";
const string LIST_FRIENDS = "6";
const string ADD_FRIEND = "7";
const string FIND_REQUEST = "8";
const string DEL_FRIEND = "9";
const string BLOCKED_LISTS = "10";
const string UNBLOCKED = "11";
const string GROUP = "12";
const string SEND_FILE = "13";
const string RECEIVE_FILE = "14";
const string REQUEST_NOTIFICATION = "15";
const string GROUP_REQUEST = "16";
const string SYNC = "17";
const string EXIT = "18";
const string BACK = "19";
/*\033[30m：黑色
\033[31m：红色
\033[32m：绿色
\033[33m：黄色
\033[34m：蓝色
\033[35m：洋红色
\033[36m：青色
\033[37m：白色*/
// 颜色代码
const std::string RESET = "\033[0m";
const std::string BLACK = "\033[30m";
const std::string RED = "\033[31m";
const std::string EXCLAMATION = "\033[1;31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";

//数据传输操作上方空一行以突出逻辑
class LoginRequest {
public:
    LoginRequest();

    //LoginRequest(const std::string &uid, const std::string &passwd) : UID(uid), passwd(passwd) {}
    LoginRequest(string uid, string passwd);

    [[nodiscard]] const string &getUID() const;

    void setUid(const string &uid);

    [[nodiscard]] const string &getPasswd() const;

    void setPasswd(const string &password);

    string to_json();

    void json_parse(const string &json);

private:
    string UID;
    string passwd;
};

struct Message {
public:
    Message();

    Message(string username, string UID_from, string UID_to, string groupName = "1");

    [[nodiscard]] string getUsername() const;

    void setUsername(const string &name);

    [[nodiscard]] const string &getUidFrom() const;

    void setUidFrom(const string &uidFrom);

    [[nodiscard]] const string &getUidTo() const;

    void setUidTo(const string &uidTo);

    [[nodiscard]] string getContent() const;

    void setContent(const string &msg);

    [[nodiscard]] const string &getGroupName() const;

    void setGroupName(const string &groupName);

    [[nodiscard]] string getTime() const;

    void setTime(const string &t);

    static string get_time();

    string to_json();

    void json_parse(const string &json);

private:
    string timeStamp;
    string username;
    string UID_from;
    string UID_to;
    //实际聊天内容
    string content;
    string group_name;
};

//服务器消息包
struct Response {
    int status;
    string prompt;
};

#endif //CHATROOM_PROTO_H
