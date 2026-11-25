#include "cinema_data_store.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error( \
    std::string("Line ") + std::to_string(__LINE__) + ": " + #condition); } while (false)

int main() {
    namespace fs = std::filesystem;
    const fs::path root = fs::current_path() / "snapshot_test_data";
    fs::remove_all(root);
    CinemaDataStore store;
    MovieCatalog movies;
    movies.addMovie(Movie(1, "Phim Một", 90, "Demo", 50000));
    std::vector<CinemaRoom> rooms{CinemaRoom(1, "Phòng Một", {Seat('A', 1)})};
    ScheduleManager schedule;
    schedule.addShowtime(Showtime(1, 1, 1, "2026-12-01 19:00", "2026-12-01 20:30", 50000));
    UserManager users;
    users.addCustomer(Customer(1, "Khách Một"));
    std::vector<Ticket> tickets;
    std::string error;
    try {
        CHECK(store.saveAll(movies, rooms, schedule, users, tickets, root.u8string(), error));
        const std::string original = [&] {
            std::ifstream in(root / "customers.txt", std::ios::binary);
            return std::string(std::istreambuf_iterator<char>(in), {});
        }();
        users.addCustomer(Customer(2, "Khách Hai"));
        CHECK(store.saveAll(movies, rooms, schedule, users, tickets, root.u8string(), error));
        std::ifstream previous(root / ".cinema-backup" / "customers.txt", std::ios::binary);
        CHECK(std::string(std::istreambuf_iterator<char>(previous), {}) == original);
        previous.close();

        // A record which cannot be loaded must fail in staging without
        // touching the current save.
        tickets.emplace_back(1, 1, 1, "A1", 50000, static_cast<TicketStatus>(99));
        CHECK(!store.saveAll(movies, rooms, schedule, users, tickets, root.u8string(), error));
        tickets.clear();
        MovieCatalog loadedMovies;
        std::vector<CinemaRoom> loadedRooms;
        ScheduleManager loadedSchedule;
        UserManager loadedUsers;
        std::vector<Ticket> loadedTickets;
        CHECK(store.loadAll(loadedMovies, loadedRooms, loadedSchedule,
                            loadedUsers, loadedTickets, root.u8string(), error));
        CHECK(loadedUsers.getCustomers().size() == 2);

        // Missing old file must not be silently overwritten by a new save.
        fs::remove(root / "rooms.txt");
        CHECK(!store.saveAll(loadedMovies, loadedRooms, loadedSchedule,
                             loadedUsers, loadedTickets, root.u8string(), error));
        CHECK(!fs::exists(root / "rooms.txt"));
        fs::copy_file(root / ".cinema-backup" / "rooms.txt", root / "rooms.txt");
        CHECK(store.loadAll(loadedMovies, loadedRooms, loadedSchedule,
                            loadedUsers, loadedTickets, root.u8string(), error));
        fs::remove_all(root);
        std::cout << "test_data_snapshot OK\n";
        return 0;
    } catch (const std::exception& ex) {
        fs::remove_all(root);
        std::cerr << "test_data_snapshot failed: " << ex.what() << '\n';
        return 1;
    }
}
