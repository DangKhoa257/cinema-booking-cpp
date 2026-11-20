#pragma once

#include "user.h"

class Customer : public User {
public:
    Customer(int id, std::string name);
    std::string role() const override;
    void displayInfo(std::ostream& out) const override;
};
