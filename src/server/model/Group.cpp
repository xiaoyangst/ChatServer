#include "Group.h"

Group::Group(int id, std::string name, std::string desc)
    : _id(id)
    , _name(name)
    , _desc(desc)
{}

std::string Group::getName() {return _name;}
std::string Group::getDesc() {return _desc;}
int Group::getId() {return _id;}
std::vector<GroupUser> &Group::getUsers() {return _users;}

void Group::setId(int id) {_id = id;}
void Group::setName(std::string name) {_name = name;}
void Group::setDesc(std::string desc) {_desc = desc;}
