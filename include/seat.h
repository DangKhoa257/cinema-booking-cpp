#pragma once

#include <string>

class Seat {
public:
    Seat(char row, int number);
    char getRow() const;
    int getNumber() const;
    std::string getSeatId() const;

private:
    char row_;
    int number_;
};
