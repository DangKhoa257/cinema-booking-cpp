#pragma once

#include "movie.h"
#include "ticket.h"
#include <string>
#include <vector>

class FileStorage {
public:
    bool saveMovies(const std::vector<Movie>& movies, const std::string& path);
    bool loadMovies(std::vector<Movie>& movies, const std::string& path);
    bool saveTickets(const std::vector<Ticket>& tickets, const std::string& path);
    bool loadTickets(std::vector<Ticket>& tickets, const std::string& path);
};
