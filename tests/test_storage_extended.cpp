#include "file_storage.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream(path, std::ios::binary) << content;
}

int main() {
    FileStorage storage;
    const std::string roomsPath = "extended_rooms.txt";
    const std::string showsPath = "extended_showtimes.txt";
    const std::string customersPath = "extended_customers.txt";

    std::vector<CinemaRoom> rooms{
        CinemaRoom(1, "Phong Mot", {Seat('A', 1), Seat('B', 12)}),
        CinemaRoom(2, "Phong Hai", {Seat('C', 3)})};
    std::vector<Showtime> shows{
        Showtime(1, 7, 1, "2026-12-01 19:00", "2026-12-01 20:30", 50000),
        Showtime(2, 7, 1, "2026-12-01 20:30", "2026-12-01 21:30", 60000)};
    std::vector<Customer> customers{Customer(1, "Khach Demo"), Customer(2, "Khach Moi")};

    assert(storage.saveRooms(rooms, roomsPath));
    assert(storage.saveShowtimes(shows, showsPath));
    assert(storage.saveCustomers(customers, customersPath));

    std::vector<CinemaRoom> loadedRooms;
    std::vector<Showtime> loadedShows;
    std::vector<Customer> loadedCustomers;
    assert(storage.loadRooms(loadedRooms, roomsPath));
    assert(storage.loadShowtimes(loadedShows, showsPath));
    assert(storage.loadCustomers(loadedCustomers, customersPath));
    assert(loadedRooms.size() == 2 && loadedRooms[0].hasSeat("B12"));
    assert(loadedShows.size() == 2 && loadedShows[1].getPriceVnd() == 60000);
    assert(loadedCustomers.size() == 2 && loadedCustomers[1].getName() == "Khach Moi");

    const char* badRooms[] = {
        "1|Phong|A1,A1\n", "1|Phong|A0\n", "1|Phong|A01\n",
        "1|Phong|A1,\n", "1|Phong|A1\n1|Trung|B1\n"};
    for (const char* content : badRooms) {
        writeFile(roomsPath, content);
        assert(!storage.loadRooms(loadedRooms, roomsPath));
        assert(loadedRooms.size() == 2 && loadedRooms[0].hasSeat("B12"));
        assert(!storage.lastError().empty());
    }
    const char* badShows[] = {
        "1|7|1|2026-12-01 19:00|2026-12-01 18:00|50000\n",
        "1|7|1|2026-12-01 19:00|2026-12-01 20:30|50000\n"
        "2|7|1|2026-12-01 20:00|2026-12-01 21:00|50000\n"};
    for (const char* content : badShows) {
        writeFile(showsPath, content);
        assert(!storage.loadShowtimes(loadedShows, showsPath));
        assert(loadedShows.size() == 2 && loadedShows[1].getId() == 2);
    }
    writeFile(customersPath, "1|Hop Le\n1|Trung ID\n");
    assert(!storage.loadCustomers(loadedCustomers, customersPath));
    assert(loadedCustomers.size() == 2 && loadedCustomers[1].getId() == 2);
    writeFile(customersPath, "3|   \n");
    assert(!storage.loadCustomers(loadedCustomers, customersPath));
    assert(loadedCustomers.size() == 2);
    assert(!storage.loadRooms(loadedRooms, "missing_extended_rooms.txt"));
    assert(loadedRooms.size() == 2);

    std::filesystem::remove(roomsPath);
    std::filesystem::remove(showsPath);
    std::filesystem::remove(customersPath);
    std::cout << "test_storage_extended OK\n";
}
