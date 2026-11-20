#pragma once

#include "user.h"

class Admin : public User {
public:
    Admin(int id, std::string name);
    std::string role() const override;
    void displayInfo(std::ostream& out) const override;
};
