#pragma once

#include <iosfwd>
#include <string>

// Ghe VAT LY trong phong. Khong luu trang thai dat ve (trang thai theo suat chieu
// do BookingManager suy ra tu Ticket).
class Seat {
public:
    // Nem std::invalid_argument neu row ngoai 'A'..'Z' hoac number < 1.
    Seat(char row, int number);
    char getRow() const;
    int getNumber() const;
    std::string getSeatId() const;  // vd "A1"

private:
    char row_;
    int number_;
};

// Chi de in/hien thi (khong dung de luu file).
std::ostream& operator<<(std::ostream& out, const Seat& seat);
