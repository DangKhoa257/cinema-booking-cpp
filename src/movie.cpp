#include "movie.h"
#include <ostream>
#include <utility>

Movie::Movie(int id, std::string title, int durationMinutes, std::string genre,
             long long basePriceVnd)
    : id_(id), title_(std::move(title)), durationMinutes_(durationMinutes),
      genre_(std::move(genre)), basePriceVnd_(basePriceVnd) {}
int Movie::getId() const { return id_; }
const std::string& Movie::getTitle() const { return title_; }
int Movie::getDurationMinutes() const { return durationMinutes_; }
const std::string& Movie::getGenre() const { return genre_; }
long long Movie::getBasePriceVnd() const { return basePriceVnd_; }
std::ostream& operator<<(std::ostream& out, const Movie&) {
    // TODO(TV2): Dinh dang thong tin phim de hien thi.
    return out << "[Movie: chuc nang dang duoc phat trien]";
}
