#include "movie.h"

#include <ostream>
#include <stdexcept>
#include <utility>

namespace {
bool isBlank(const std::string& value) {
    return value.find_first_not_of(" \t\n\r\f\v") == std::string::npos;
}
}

Movie::Movie(int id, std::string title, int durationMinutes, std::string genre,
             long long basePriceVnd)
    : id_(id), title_(std::move(title)), durationMinutes_(durationMinutes),
      genre_(std::move(genre)), basePriceVnd_(basePriceVnd) {
    if (id_ <= 0) {
        throw std::invalid_argument("Movie ID must be positive");
    }
    if (isBlank(title_)) {
        throw std::invalid_argument("Movie title must not be empty");
    }
    if (durationMinutes_ <= 0) {
        throw std::invalid_argument("Movie duration must be greater than 0");
    }
    if (isBlank(genre_)) {
        throw std::invalid_argument("Movie genre must not be empty");
    }
    if (basePriceVnd_ < 0) {
        throw std::invalid_argument("Movie base price must not be negative");
    }
}

int Movie::getId() const { return id_; }
const std::string& Movie::getTitle() const { return title_; }
int Movie::getDurationMinutes() const { return durationMinutes_; }
const std::string& Movie::getGenre() const { return genre_; }
long long Movie::getBasePriceVnd() const { return basePriceVnd_; }

std::ostream& operator<<(std::ostream& out, const Movie& movie) {
    out << "ID: " << movie.getId()
        << " | Ten phim: " << movie.getTitle()
        << " | Thoi luong: " << movie.getDurationMinutes() << " phut"
        << " | The loai: " << movie.getGenre()
        << " | Gia ve co ban: " << movie.getBasePriceVnd() << " VND";
    return out;
}