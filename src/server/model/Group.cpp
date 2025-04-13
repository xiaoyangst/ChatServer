#include "Group.h"

#include <utility>

Group::Group(int id, std::string name, std::string desc)
    : _id(id)
    , _name(std::move(name))
    , _desc(std::move(desc))
{}

std::string Group::getName() {return _name;}
std::string Group::getDesc() {return _desc;}
int Group::getId() const {return _id;}
std::vector<GroupUser> &Group::getUsers() {return _users;}

void Group::setId(int id) {_id = id;}
void Group::setName(std::string name) {_name = std::move(name);}
void Group::setDesc(std::string desc) {_desc = std::move(desc);}
