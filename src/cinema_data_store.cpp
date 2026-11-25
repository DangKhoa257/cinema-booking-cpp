#include "cinema_data_store.h"
#include <exception>
#include <filesystem>
#include <set>
#include <utility>

namespace {
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
        if (!storage_.saveMovies(movies.getMovies(), (root / "movies.txt").u8string()) ||
            !storage_.saveRooms(rooms, (root / "rooms.txt").u8string()) ||
            !storage_.saveShowtimes(schedule.getShowtimes(), (root / "showtimes.txt").u8string()) ||
            !storage_.saveCustomers(users.getCustomers(), (root / "customers.txt").u8string()) ||
            !storage_.saveTickets(tickets, (root / "tickets.txt").u8string())) {
            error = storage_.lastError();
            return false;
        }
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
