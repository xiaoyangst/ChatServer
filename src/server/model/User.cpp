#include "User.h"

#include <utility>

User::User(int id, std::string name, std::string pwd, std::string state)
        : _id(id), _name(std::move(name)), _password(std::move(pwd)), _state(std::move(state)) {}

void User::setId(const int id) {
    _id = id;
}

void User::setName(const std::string name) {
    _name = name;
}

void User::setPassword(const std::string pwd) {
    _password = pwd;
}

void User::setState(const std::string state) {
    _state = state;
}

int User::getId() { return _id; }

std::string User::getName() { return _name; }

std::string User::getPassword() { return _password; }

std::string User::getState() { return _state; }