#include "seat.h"
#include <ostream>
#include <stdexcept>

Seat::Seat(char row, int number) : row_(row), number_(number) {
    if (row_ < 'A' || row_ > 'Z') {
        throw std::invalid_argument("Seat row must be 'A'..'Z'");
    }
    if (number_ < 1) {
        throw std::invalid_argument("Seat number must be >= 1");
    }
}
char Seat::getRow() const { return row_; }
int Seat::getNumber() const { return number_; }
std::string Seat::getSeatId() const { return std::string(1, row_) + std::to_string(number_); }

std::ostream& operator<<(std::ostream& out, const Seat& seat) {
    return out << seat.getSeatId();
}
