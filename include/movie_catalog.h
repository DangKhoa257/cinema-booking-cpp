#pragma once

#include "movie.h"
#include <vector>

class MovieCatalog {
public:
    bool addMovie(const Movie& movie);
    const Movie* findMovie(int id) const;
    const std::vector<Movie>& getMovies() const;
    bool removeMovie(int id);

private:
    std::vector<Movie> movies_;
};
