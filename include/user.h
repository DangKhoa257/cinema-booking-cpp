#pragma once

#include <iosfwd>
#include <string>

class User {
public:
    User(int id, std::string name);
    virtual ~User();
    int getId() const;
    const std::string& getName() const;
    virtual std::string role() const = 0;
    virtual void displayInfo(std::ostream& out) const = 0;

private:
    int id_;
    std::string name_;
};
