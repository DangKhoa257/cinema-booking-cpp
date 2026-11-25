#pragma once

#include "seat.h"
#include <iosfwd>
#include <string>
#include <vector>

// Phong chieu: so do ghe VAT LY. Quan he has-a (composition): phong so huu cac Seat.
class CinemaRoom {
public:
    // Nem std::invalid_argument neu id <= 0, ten rong/chua '|' hoac xuong dong,
    // danh sach ghe rong hoac co seatId trung.
    CinemaRoom(int id, std::string name, std::vector<Seat> seats);
    int getId() const;
    const std::string& getName() const;
    const std::vector<Seat>& getSeats() const;
    bool hasSeat(const std::string& seatId) const;

private:
    int id_;
    std::string name_;
    std::vector<Seat> seats_;
};

// Bo sung (chi them, can TV1 dong y): in so do ghe cua phong de kiem thu/hien thi.
std::ostream& operator<<(std::ostream& out, const CinemaRoom& room);
