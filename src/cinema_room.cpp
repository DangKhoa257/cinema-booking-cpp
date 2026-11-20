#include "cinema_room.h"
#include <utility>

CinemaRoom::CinemaRoom(int id, std::string name, std::vector<Seat> seats)
    : id_(id), name_(std::move(name)), seats_(std::move(seats)) {}
int CinemaRoom::getId() const { return id_; }
const std::string& CinemaRoom::getName() const { return name_; }
const std::vector<Seat>& CinemaRoom::getSeats() const { return seats_; }
bool CinemaRoom::hasSeat(const std::string&) const {
    // TODO(TV3): Tim seatId trong so do phong.
    return false;
}
