#include "cinema_data_store.h"
#include <exception>
#include <filesystem>
#include <set>
#include <utility>
#include <array>

namespace {
constexpr std::array<const char*, 5> dataFiles = {
    "movies.txt", "rooms.txt", "showtimes.txt", "customers.txt", "tickets.txt"};

// Keep the previous complete set until all five new files are installed.
// A failed replacement restores the previous files from this directory.
bool restoreFiles(const std::filesystem::path& root,
                  const std::filesystem::path& backup,
                  const std::array<bool, 5>& existed, std::string& error) {
    bool restored = true;
    for (size_t i = 0; i < dataFiles.size(); ++i) {
        std::error_code ec;
        std::filesystem::remove(root / dataFiles[i], ec);
        if (ec) restored = false;
        ec.clear();
        if (existed[i])
            std::filesystem::copy_file(backup / dataFiles[i], root / dataFiles[i],
                ec);
        if (ec) restored = false;
    }
    if (!restored) error += " Khong khoi phuc duoc tat ca file; xem thu muc .cinema-backup.";
    return restored;
}

bool validReferences(const MovieCatalog& movies,
                     const std::vector<CinemaRoom>& rooms,
                     const ScheduleManager& schedule, const UserManager& users,
                     const std::vector<Ticket>& tickets, std::string& error) {
    std::set<int> roomIds;
    for (const CinemaRoom& room : rooms) {
        if (!roomIds.insert(room.getId()).second) {
            error = "Trung ID phong chieu.";
            return false;
        }
    }
    for (const Showtime& show : schedule.getShowtimes()) {
        if (!movies.findMovie(show.getMovieId()) ||
            !roomIds.count(show.getRoomId())) {
            error = "Suat chieu tham chieu phim hoac phong khong ton tai.";
            return false;
        }
    }
    std::set<std::pair<int, std::string>> activeSeats;
    for (const Ticket& ticket : tickets) {
        const Showtime* show = schedule.findShowtime(ticket.getShowtimeId());
        if (!show || !users.findCustomer(ticket.getCustomerId()) ||
            ticket.getPriceVnd() != show->getPriceVnd()) {
            error = "Ve tham chieu khach, suat hoac gia khong hop le.";
            return false;
        }
        bool hasSeat = false;
        for (const CinemaRoom& room : rooms)
            if (room.getId() == show->getRoomId() && room.hasSeat(ticket.getSeatId()))
                hasSeat = true;
        if (!hasSeat) {
            error = "Ve tham chieu ghe khong ton tai trong phong.";
            return false;
        }
        if (ticket.getStatus() != TicketStatus::Cancelled &&
            !activeSeats.insert({ticket.getShowtimeId(), ticket.getSeatId()}).second) {
            error = "Trung ghe dang hoat dong trong cung suat chieu.";
            return false;
        }
    }
    return true;
}
}  // namespace

bool CinemaDataStore::saveAll(const MovieCatalog& movies,
                              const std::vector<CinemaRoom>& rooms,
                              const ScheduleManager& schedule,
                              const UserManager& users,
                              const std::vector<Ticket>& tickets,
                              const std::string& directory, std::string& error) {
    error.clear();
    try {
        if (!validReferences(movies, rooms, schedule, users, tickets, error)) return false;
        std::error_code ec;
        const auto root = std::filesystem::u8path(directory);
        std::filesystem::create_directories(root, ec);
        if (ec) { error = "Khong tao duoc thu muc: " + ec.message(); return false; }
        const auto staging = root / ".cinema-next";
        const auto backup = root / ".cinema-backup";
        std::filesystem::remove_all(staging, ec);
        if (ec) { error = "Khong don duoc vung ghi tam: " + ec.message(); return false; }
        std::filesystem::create_directory(staging, ec);
        if (ec) { error = "Khong tao duoc vung ghi tam: " + ec.message(); return false; }
        if (!storage_.saveMovies(movies.getMovies(), (staging / "movies.txt").u8string()) ||
            !storage_.saveRooms(rooms, (staging / "rooms.txt").u8string()) ||
            !storage_.saveShowtimes(schedule.getShowtimes(), (staging / "showtimes.txt").u8string()) ||
            !storage_.saveCustomers(users.getCustomers(), (staging / "customers.txt").u8string()) ||
            !storage_.saveTickets(tickets, (staging / "tickets.txt").u8string())) {
            error = storage_.lastError();
            return false;
        }
        // Some individual save APIs do not reject every character that their
        // matching loader rejects. Verify the complete staged set before use.
        MovieCatalog checkedMovies;
        std::vector<CinemaRoom> checkedRooms;
        ScheduleManager checkedSchedule;
        UserManager checkedUsers;
        std::vector<Ticket> checkedTickets;
        if (!loadAll(checkedMovies, checkedRooms, checkedSchedule, checkedUsers,
                     checkedTickets, staging.u8string(), error)) return false;
        std::array<bool, 5> existed{};
        for (size_t i = 0; i < dataFiles.size(); ++i) {
            existed[i] = std::filesystem::exists(root / dataFiles[i]);
            if (!existed[i]) {
                // A partial old set is kept intact until the user repairs it.
                for (const char* name : dataFiles)
                    if (std::filesystem::exists(root / name)) {
                        error = "Ban luu cu thieu file; khong tu ghi de.";
                        return false;
                    }
            }
        }
        std::filesystem::remove_all(backup, ec);
        if (ec) { error = "Khong don duoc ban sao luu: " + ec.message(); return false; }
        std::filesystem::create_directory(backup, ec);
        if (ec) { error = "Khong tao duoc ban sao luu: " + ec.message(); return false; }
        for (size_t i = 0; i < dataFiles.size(); ++i) {
            if (!existed[i]) continue;
            std::filesystem::copy_file(root / dataFiles[i], backup / dataFiles[i], ec);
            if (ec) { error = "Sao luu that bai: " + ec.message(); return false; }
        }
        for (const char* name : dataFiles) {
            std::filesystem::remove(root / name, ec);
            if (!ec) std::filesystem::copy_file(staging / name, root / name, ec);
            if (ec) {
                error = "Thay ban luu that bai: " + ec.message();
                restoreFiles(root, backup, existed, error);
                return false;
            }
        }
        std::filesystem::remove_all(staging, ec); // A cleanup failure does not invalidate the save.
        return true;
    } catch (const std::exception& ex) {
        error = std::string("Duong dan luu khong hop le: ") + ex.what();
        return false;
    }
}

bool CinemaDataStore::loadAll(MovieCatalog& movies,
                              std::vector<CinemaRoom>& rooms,
                              ScheduleManager& schedule, UserManager& users,
                              std::vector<Ticket>& tickets,
                              const std::string& directory, std::string& error) {
    error.clear();
    try {
        const auto root = std::filesystem::u8path(directory);
        std::vector<Movie> loadedMovies;
        std::vector<CinemaRoom> loadedRooms;
        std::vector<Showtime> loadedShows;
        std::vector<Customer> loadedCustomers;
        std::vector<Ticket> loadedTickets;
        if (!storage_.loadMovies(loadedMovies, (root / "movies.txt").u8string()) ||
            !storage_.loadRooms(loadedRooms, (root / "rooms.txt").u8string()) ||
            !storage_.loadShowtimes(loadedShows, (root / "showtimes.txt").u8string()) ||
            !storage_.loadCustomers(loadedCustomers, (root / "customers.txt").u8string()) ||
            !storage_.loadTickets(loadedTickets, (root / "tickets.txt").u8string())) {
            error = storage_.lastError();
            return false;
        }

        MovieCatalog newMovies;
        ScheduleManager newSchedule;
        UserManager newUsers;
        for (const Movie& movie : loadedMovies)
            if (!newMovies.addMovie(movie)) { error = "Trung ID phim."; return false; }
        for (const Customer& customer : loadedCustomers)
            if (!newUsers.addCustomer(customer)) { error = "Trung ID khach."; return false; }
        for (const Showtime& show : loadedShows)
            if (!newSchedule.addShowtime(show)) { error = "Lich chieu khong hop le."; return false; }
        if (!validReferences(newMovies, loadedRooms, newSchedule, newUsers,
                             loadedTickets, error)) return false;

        movies = std::move(newMovies);
        rooms = std::move(loadedRooms);
        schedule = std::move(newSchedule);
        users = std::move(newUsers);
        tickets = std::move(loadedTickets);
        return true;
    } catch (const std::exception& ex) {
        error = std::string("Duong dan tai khong hop le: ") + ex.what();
        return false;
    }
}
