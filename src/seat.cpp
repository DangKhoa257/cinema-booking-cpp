#include "seat.h"

Seat::Seat(char row, int number) : row_(row), number_(number) {}
char Seat::getRow() const { return row_; }
int Seat::getNumber() const { return number_; }
std::string Seat::getSeatId() const { return std::string(1, row_) + std::to_string(number_); }
