#include "cinema_room.h"
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>
#include <utility>

namespace {
bool isBlank(const std::string& s) {
    return s.find_first_not_of(" \t\n\r\f\v") == std::string::npos;
}
}  // namespace

CinemaRoom::CinemaRoom(int id, std::string name, std::vector<Seat> seats)
    : id_(id), name_(std::move(name)), seats_(std::move(seats)) {
    if (id_ <= 0) {
        throw std::invalid_argument("Room ID must be positive");
    }
    if (isBlank(name_)) {
        throw std::invalid_argument("Room name must not be empty");
    }
    if (name_.find_first_of("|\n\r") != std::string::npos) {
        throw std::invalid_argument("Room name must not contain '|', '\\n' or '\\r'");
    }
    if (seats_.empty()) {
        throw std::invalid_argument("Room must have at least one seat");
    }
    std::set<std::string> seen;
    for (const Seat& seat : seats_) {
        if (!seen.insert(seat.getSeatId()).second) {
            throw std::invalid_argument("Duplicate seatId in room: " + seat.getSeatId());
        }
    }
}
int CinemaRoom::getId() const { return id_; }
const std::string& CinemaRoom::getName() const { return name_; }
const std::vector<Seat>& CinemaRoom::getSeats() const { return seats_; }

// Chi xac nhan vi tri ghe vat ly ton tai (khong xet trang thai dat).
bool CinemaRoom::hasSeat(const std::string& seatId) const {
    for (const Seat& seat : seats_) {
        if (seat.getSeatId() == seatId) return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& out, const CinemaRoom& room) {
    out << "Phong " << room.getId() << ": " << room.getName() << "\n";
    std::map<char, std::vector<int>> rows;  // hang -> danh sach so ghe
    for (const Seat& seat : room.getSeats()) rows[seat.getRow()].push_back(seat.getNumber());
    for (const auto& [row, numbers] : rows) {
        out << row << ":";
        for (int n : numbers) out << ' ' << row << n;
        out << "\n";
    }
    return out;
}
