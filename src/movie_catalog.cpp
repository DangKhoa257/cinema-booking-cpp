#include "movie_catalog.h"

#include <algorithm>

bool MovieCatalog::addMovie(const Movie& movie) {
    if (movie.getId() <= 0 || findMovie(movie.getId()) != nullptr) {
        return false;
    }

    movies_.push_back(movie);
    return true;
}

const Movie* MovieCatalog::findMovie(int id) const {
    if (id <= 0) {
        return nullptr;
    }

    for (const Movie& movie : movies_) {
        if (movie.getId() == id) {
            return &movie;
        }
    }
    return nullptr;
}

const std::vector<Movie>& MovieCatalog::getMovies() const {
    return movies_;
}

bool MovieCatalog::removeMovie(int id) {
    for (auto it = movies_.begin(); it != movies_.end(); ++it) {
        if (it->getId() == id) {
            movies_.erase(it);
            return true;
        }
    }
    return false;
}