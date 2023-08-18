//
// Created by shawn on 23-8-13.
//

#include <json/json.h>

#include <utility>
#include <random>
#include <iostream>
#include "Group.h"

using namespace std;
using namespace Json;

Group::Group(string groupName, string UID) : groupName(std::move(groupName)), UID(std::move(UID)) {
    random_device rd;
    mt19937 eng(rd());
    uniform_int_distribution<> dist(10, 99);
    int random_num = dist(eng);

    time_t timer;
    time(&timer);
    //cout<<"timer: "<<timer<<endl;
    string timeStamp = to_string(timer).substr(8, 2);
    //cout<<"timeStamp: "<<timeStamp<<endl;
    groupUID = to_string(random_num).append(timeStamp);
    admins = groupUID + "admin";
    members = groupUID + "member";
}

std::string Group::to_json() {
    Value root;
    root["groupName"] = groupName;
    root["UID"] = UID;
    root["groupUID"] = groupUID;
    root["members"] = members;
    root["admins"] = admins;
    FastWriter writer;
    return writer.write(root);
}

void Group::json_parse(const std::string &json) {
    Value root;
    Reader reader;
    reader.parse(json, root);
    groupName = root["groupName"].asString();
    UID = root["UID"].asString();
    groupUID = root["groupUID"].asString();
    members = root["members"].asString();
    admins = root["admins"].asString();
}

const string &Group::getGroupName() const {
    return groupName;
}

const string &Group::getGroupUid() const {
    return groupUID;
}

const string &Group::getMembers() const {
    return members;
}

const string &Group::getAdmins() const {
    return admins;
}

const std::string &Group::getOwnerUid() const {
    return UID;
}


