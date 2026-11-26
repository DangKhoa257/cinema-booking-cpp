#include "cinema_v2.h"
#include "booking_manager.h"
#include "cinema_data_store.h"
#include "demo_payment.h"
#include "bank_transfer_payment.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>

namespace {
bool check(bool ok, const char* label) {
    if (!ok) std::cerr << label << '\n';
    return ok;
}
}  // namespace

int main() {
    std::string error;
    MovieDetailsCatalog movies;
    if (!check(movies.load(MOVIE_DATA_FILE, error), "load real movie metadata")) return 1;
    CinemaV2 cinema;
    if (!check(cinema.load(CINEMA_DATA_FILE, movies, error, "2030-01-01 12:00"),
               "load relative cinema demo")) { std::cerr << error << '\n'; return 1; }
    if (!check(cinema.branches().all().size() == 4 && cinema.rooms().size() == 12 &&
               cinema.schedule().getShowtimes().size() == 32 &&
               cinema.cleaningMinutes() == 15, "demo branch/room/show counts")) return 1;
    size_t totalSeats = 0;
    for (const auto& branch : cinema.branches().all())
        if (!check(branch.roomIds.size() == 3 && branch.active,
                   "three active rooms per branch")) return 1;
    for (const auto& room : cinema.rooms()) {
        const auto* layout = cinema.layout(room.getId());
        const auto* branch = cinema.branches().findByRoom(room.getId());
        if (!check(layout && branch && layout->positions().size() == room.getSeats().size() &&
                   room.getSeats().size() >= 80 && room.getSeats().size() <= 120,
                   "valid room/layout/branch relation")) return 1;
        std::set<std::string> seatIds;
        bool hasStandard = false, hasVip = false;
        for (const auto& seat : layout->positions()) {
            seatIds.insert(seat.seatId);
            hasStandard |= seat.category == SeatCategory::Standard;
            hasVip |= seat.category == SeatCategory::VIP;
            if (!check(seat.column != layout->aisleAfter() + 1 && room.hasSeat(seat.seatId),
                       "aisle has no bookable seat")) return 1;
        }
        if (!check(seatIds.size() == layout->positions().size() && hasStandard && hasVip,
                   "seat IDs and categories")) return 1;
        totalSeats += seatIds.size();
    }
    if (!check(totalSeats == 1144 && cinema.layout(101)->find("A6")->column == 7 &&
               cinema.layout(101)->find("G1")->category == SeatCategory::VIP,
               "physical layout coordinates")) return 1;
    if (!check(cinema.showtimesFor(101, 1, "2030-01-02").size() == 1 &&
               cinema.showtimesFor(101, 2, "2030-01-02").size() == 1,
               "filter showtimes by movie, branch and day")) return 1;

    const auto* first = cinema.schedule().findShowtime(1);
    if (!check(first && first->getStartTime() == "2030-01-02 10:00" &&
               first->getEndTime() == "2030-01-02 12:15", "movie duration used")) return 1;
    const std::string firstStart = first->getStartTime();
    if (!check(cinema.isBookable(1, "2030-01-02 09:59") &&
               !cinema.isBookable(1, "2030-01-02 10:00") &&
               !cinema.isBookable(1, "2030-01-03 10:00"), "past show gate")) return 1;
    if (!check(!cinema.addShowtime(Showtime(90, 101, 101, "2030-01-02 12:20",
                                            "2030-01-02 14:35", 80000), movies, error),
               "cleaning buffer blocks adjacent show")) return 1;
    if (!check(cinema.addShowtime(Showtime(90, 101, 101, "2030-01-04 10:00",
                                           "2030-01-04 12:15", 80000), movies, error) &&
               cinema.addShowtime(Showtime(91, 101, 102, "2030-01-04 10:00",
                                           "2030-01-04 12:15", 80000), movies, error),
               "same time in different rooms")) return 1;
    if (!check(!cinema.addShowtime(Showtime(92, 101, 101, "2030-01-04 11:00",
                                            "2030-01-04 13:15", 80000), movies, error),
               "same room overlap rejected")) return 1;

    UserManager users;
    users.addCustomer(Customer(1, "Khách thử"));
    std::vector<Ticket> tickets;
    BookingManager booking(cinema.schedule(), cinema.rooms(), users, tickets);
    DemoPayment paid;
    BankTransferPayment pending;
    if (!check(!cinema.bookTicket(booking, 1, 1, "A1", paid, "2030-01-02 10:00") &&
               tickets.empty(), "past show cannot create ticket")) return 1;
    if (!check(cinema.bookTicket(booking, 1, 1, "A1", pending, "2030-01-02 09:00") &&
               booking.getSeatStatus(1, "A1") == SeatStatus::Held,
               "pending seat held")) return 1;
    if (!check(!cinema.bookTicket(booking, 1, 1, "A1", paid, "2030-01-02 09:00") &&
               cinema.bookTicket(booking, 1, 2, "A1", paid, "2030-01-02 09:00") &&
               booking.getSeatStatus(2, "A1") == SeatStatus::Booked,
               "duplicate blocked, same seat other show allowed")) return 1;
    if (!check(booking.cancelTicket(tickets.front().getId()) &&
               booking.getSeatStatus(1, "A1") == SeatStatus::Available,
               "cancel releases seat only in that show")) return 1;

    const auto root = std::filesystem::temp_directory_path() /
                      std::filesystem::u8path("oop_rạp_phòng_test");
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);
    if (!check(!ec, "Unicode directory")) return 1;
    const auto file = root / std::filesystem::u8path("cụm_rạp.txt");
    if (!check(cinema.save(file.u8string(), error), "save cinema snapshot")) return 1;
    CinemaV2 restored;
    if (!restored.load(file.u8string(), movies, error, "2040-01-01 12:00")) {
        std::cerr << "round trip load: " << error << '\n';
        return 1;
    }
    if (!check(restored.schedule().getShowtimes().size() == 34 &&
               restored.schedule().findShowtime(1)->getStartTime() == firstStart,
               "round trip stores absolute showtimes")) return 1;
    if (!check(restored.save(file.u8string(), error), "second save")) return 1;
    auto backup = file; backup += ".bak";
    if (!check(std::filesystem::is_regular_file(backup), "previous snapshot backed up")) return 1;
    const auto corrupt = root / "corrupt.txt";
    { std::ofstream out(corrupt, std::ios::binary); out << "C|15\nB|1|Thiếu|X|Y|1\nR|1|99|P|10|10|5|7\n"; }
    if (!check(!restored.load(corrupt.u8string(), movies, error) &&
               restored.rooms().size() == 12, "orphan room leaves memory intact")) return 1;
    { std::ofstream out(corrupt, std::ios::binary); out << "C|15\nB|1|Lỗi|X|Y|1\nB|1|Trùng|X|Y|1\n"; }
    if (!check(!restored.load(corrupt.u8string(), movies, error) &&
               restored.branches().find(1)->name == "OOP Cinema Thủ Đức",
               "duplicate branch leaves memory intact")) return 1;
    { std::ifstream in(CINEMA_DATA_FILE, std::ios::binary);
      std::ofstream out(corrupt, std::ios::binary);
      out << in.rdbuf() << "R|999|1|Phòng sai|2|2|1|2\n"; }
    if (!check(!restored.load(corrupt.u8string(), movies, error,
                              "2030-01-01 12:00") && restored.rooms().size() == 12,
               "invalid layout leaves memory intact")) return 1;
    { std::ifstream in(CINEMA_DATA_FILE, std::ios::binary);
      std::ofstream out(corrupt, std::ios::binary);
      out << in.rdbuf() << "S|999|101|101|2030-01-02 12:20|2030-01-02 14:35|80000\n"; }
    if (!check(!restored.load(corrupt.u8string(), movies, error,
                              "2030-01-01 12:00") &&
               restored.schedule().getShowtimes().size() == 34,
               "overlapping persisted show leaves memory intact")) return 1;
    const auto blockedTarget = root / "directory-instead-of-file";
    std::filesystem::create_directory(blockedTarget, ec);
    if (!check(!restored.save(blockedTarget.u8string(), error) &&
               std::filesystem::is_directory(blockedTarget),
               "failed save preserves target")) return 1;

    // Năm file V1 vẫn đọc bằng API cũ, không cần V2 trong thư mục đó.
    CinemaDataStore legacyStore;
    MovieCatalog legacyMovies;
    std::vector<CinemaRoom> legacyRooms;
    ScheduleManager legacySchedule;
    UserManager legacyUsers;
    std::vector<Ticket> legacyTickets;
    if (!check(legacyStore.loadAll(legacyMovies, legacyRooms, legacySchedule,
                                   legacyUsers, legacyTickets, LEGACY_DATA_DIR, error) &&
               legacyRooms.size() == 1 && legacyTickets.size() == 1,
               "V1 snapshot compatibility")) { std::cerr << error << '\n'; return 1; }
    std::filesystem::remove_all(root, ec);
    return 0;
}
