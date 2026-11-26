#include "movie_details_catalog.h"
#include "file_storage.h"

#include <filesystem>
#include <iostream>
#include <set>

int main() {
    MovieDetailsCatalog catalog;
    std::string error;
    if (!catalog.load(MOVIE_DATA_FILE, error)) {
        std::cerr << error << '\n';
        return 1;
    }
    if (catalog.all().size() != 11 || catalog.nowShowing().size() != 10 ||
        catalog.comingSoon().size() != 1 ||
        catalog.searchTitle("Godzilla").size() != 1 ||
        catalog.filterGenre("Hoạt hình").size() != 2) {
        std::cerr << "Seed catalog content or queries are wrong\n";
        return 1;
    }
    const std::set<int> posterIds{101, 102, 103, 105, 106, 107, 109, 110, 111};
    for (const auto& movie : catalog.all()) {
        if (movie.sourceUrl.empty() || movie.verifiedAt < "2026-11-25" ||
            movie.durationMinutes <= 0 ||
            bool(posterIds.count(movie.movieId)) != !movie.posterAsset.empty()) {
            std::cerr << "Unverified seed movie " << movie.movieId << '\n';
            return 1;
        }
    }
    if (!catalog.find(107) || !catalog.find(107)->ageRating.empty()) {
        std::cerr << "Missing age rating was guessed\n";
        return 1;
    }
    // V1 files and public API still load without any V2 metadata file.
    const auto v1Path = (std::filesystem::path(MOVIE_DATA_FILE).parent_path().parent_path()
                         / "movies.txt").u8string();
    FileStorage storage;
    std::vector<Movie> oldMovies;
    if (!storage.loadMovies(oldMovies, v1Path) || oldMovies.size() != 1 ||
        oldMovies.front().getTitle() != "Phim Demo") {
        std::cerr << "V1 movie file incompatible: " << storage.lastError() << '\n';
        return 1;
    }
    return 0;
}
