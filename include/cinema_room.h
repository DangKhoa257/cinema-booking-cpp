#pragma once

#include "seat.h"
#include <string>
#include <vector>

class CinemaRoom {
public:
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
