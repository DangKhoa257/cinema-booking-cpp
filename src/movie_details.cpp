#include "movie_details.h"

Movie MovieDetails::toV1Movie(long long basePriceVnd) const {
    return Movie(movieId, title, durationMinutes, genre, basePriceVnd);
}
