#pragma once

#include "movie.h"
#include "ticket.h"
#include "cinema_room.h"
#include "showtime.h"
#include "customer.h"
#include <string>
#include <vector>

class FileStorage {
public:
    bool saveMovies(const std::vector<Movie>& movies, const std::string& path);
    bool loadMovies(std::vector<Movie>& movies, const std::string& path);
    bool saveTickets(const std::vector<Ticket>& tickets, const std::string& path);
    bool loadTickets(std::vector<Ticket>& tickets, const std::string& path);
    bool saveRooms(const std::vector<CinemaRoom>& rooms, const std::string& path);
    bool loadRooms(std::vector<CinemaRoom>& rooms, const std::string& path);
    bool saveShowtimes(const std::vector<Showtime>& showtimes, const std::string& path);
    bool loadShowtimes(std::vector<Showtime>& showtimes, const std::string& path);
    bool saveCustomers(const std::vector<Customer>& customers, const std::string& path);
    bool loadCustomers(std::vector<Customer>& customers, const std::string& path);

    // Bo sung (khong doi chu ky cu): ly do loi cua lan goi save/load gan nhat,
    // rong neu lan do thanh cong. Can TV1 dong y.
    const std::string& lastError() const;

private:
    std::string lastError_;
};
