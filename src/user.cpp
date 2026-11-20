#include "user.h"
#include <utility>

User::User(int id, std::string name) : id_(id), name_(std::move(name)) {}
User::~User() = default;
int User::getId() const { return id_; }
const std::string& User::getName() const { return name_; }
