#include "cinema_room.h"
#include "seat.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

template <typename F>
static bool throwsInvalid(F f) {
    try { f(); } catch (const std::invalid_argument&) { return true; }
    return false;
}

// So do mau A1-E8 (5 hang x 8 ghe).
static std::vector<Seat> grid(int rows, int cols) {
    std::vector<Seat> seats;
    for (int r = 0; r < rows; ++r)
        for (int c = 1; c <= cols; ++c) seats.emplace_back(static_cast<char>('A' + r), c);
    return seats;
}

int main() {
    // Seat
    Seat a1('A', 1);
    assert(a1.getSeatId() == "A1" && a1.getRow() == 'A' && a1.getNumber() == 1);
    assert(Seat('Z', 12).getSeatId() == "Z12");
    assert(throwsInvalid([] { Seat('a', 1); }));
    assert(throwsInvalid([] { Seat('1', 1); }));
    assert(throwsInvalid([] { Seat('A', 0); }));
    assert(throwsInvalid([] { Seat('A', -3); }));

    // Room: so do A1-E8
    CinemaRoom room(1, "Phong 1", grid(5, 8));
    assert(room.getId() == 1 && room.getName() == "Phong 1");
    assert(room.getSeats().size() == 40);
    assert(room.hasSeat("A1") && room.hasSeat("E8"));
    assert(!room.hasSeat("F1") && !room.hasSeat("A9") && !room.hasSeat("A0"));
    assert(!room.hasSeat("") && !room.hasSeat("a1") && !room.hasSeat("A") && !room.hasSeat("??"));

    // Du lieu khong hop le bi tu choi
    assert(throwsInvalid([] { CinemaRoom(0, "P", grid(1, 1)); }));
    assert(throwsInvalid([] { CinemaRoom(-1, "P", grid(1, 1)); }));
    assert(throwsInvalid([] { CinemaRoom(1, "", grid(1, 1)); }));
    assert(throwsInvalid([] { CinemaRoom(1, "   ", grid(1, 1)); }));
    assert(throwsInvalid([] { CinemaRoom(1, "A|B", grid(1, 1)); }));
    assert(throwsInvalid([] { CinemaRoom(1, "P", std::vector<Seat>{}); }));
    assert(throwsInvalid([] { CinemaRoom(1, "P", std::vector<Seat>{Seat('A', 1), Seat('A', 1)}); }));

    // Hai phong co ghe A1 doc lap
    CinemaRoom r2(2, "Phong 2", std::vector<Seat>{Seat('A', 1), Seat('A', 2)});
    assert(r2.hasSeat("A1") && !r2.hasSeat("B1") && room.hasSeat("B1"));

    // Sao chep phong khong mat du lieu va doc lap voi ban goc
    CinemaRoom copy = room;
    assert(copy.getSeats().size() == 40 && copy.hasSeat("E8") && copy.getName() == "Phong 1");
    copy = r2;
    assert(copy.getId() == 2 && copy.getSeats().size() == 2 && room.getSeats().size() == 40);

    // Chuoi in
    std::ostringstream os;
    os << r2;
    assert(os.str().find("Phong 2") != std::string::npos && os.str().find("A2") != std::string::npos);
    std::ostringstream os2;
    os2 << a1;
    assert(os2.str() == "A1");

    std::cout << "test_room OK\n";
}
