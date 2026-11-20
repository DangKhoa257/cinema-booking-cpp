#include "movie_catalog.h"

bool MovieCatalog::addMovie(const Movie&) {
    // TODO(TV2): Kiem tra ID trung va du lieu hop le.
    return false;
}
const Movie* MovieCatalog::findMovie(int) const {
    // TODO(TV2): Tim phim theo ID.
    return nullptr;
}
const std::vector<Movie>& MovieCatalog::getMovies() const { return movies_; }
bool MovieCatalog::removeMovie(int) {
    // TODO(TV2): Xoa phim theo ID.
    return false;
}
