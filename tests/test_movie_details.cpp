#include "movie_details_catalog.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
bool check(bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
    return condition;
}

MovieDetails sample() {
    MovieDetails movie;
    movie.movieId = 10;
    movie.title = "Điện ảnh Việt";
    movie.originalTitle = "Viet Cinema";
    movie.genre = "Tâm lý, Hài";
    movie.durationMinutes = 105;
    movie.ageRating = "T16";
    movie.releaseDate = "2026-11-25";
    movie.director = "Đạo diễn A";
    movie.cast = "Diễn viên B";
    movie.synopsis = "Câu chuyện có dấu | gạch chéo \\ và dòng\nmới";
    movie.availability = MovieAvailability::NowShowing;
    movie.sourceUrl = "https://example.org/movie";
    movie.verifiedAt = "2026-11-25";
    return movie;
}
}  // namespace

int main() {
    std::string error;
    MovieDetailsCatalog catalog;
    MovieDetails movie = sample();
    if (!check(catalog.add(movie, error), "add valid movie")) return 1;
    if (!check(!catalog.add(movie, error), "duplicate ID rejected")) return 1;
    if (!check(catalog.find(10) && catalog.find(10)->title == movie.title,
               "find by ID")) return 1;
    if (!check(catalog.nowShowing().size() == 1 && catalog.comingSoon().empty(),
               "status filters")) return 1;
    if (!check(catalog.searchTitle("VIET").size() == 1 &&
               catalog.searchTitle("Điện").size() == 1 &&
               catalog.filterGenre("Hài").size() == 1, "search and genre filter")) return 1;

    MovieDetails upcoming = movie;
    upcoming.movieId = 11;
    upcoming.title = "Phim sắp chiếu";
    upcoming.availability = MovieAvailability::ComingSoon;
    upcoming.ageRating.clear();
    if (!check(catalog.add(upcoming, error) && catalog.comingSoon().size() == 1,
               "optional age rating and coming soon")) return 1;

    MovieDetails invalid = movie;
    invalid.movieId = 12;
    invalid.releaseDate = "2026-02-30";
    if (!check(!catalog.add(invalid, error), "invalid date rejected")) return 1;
    invalid = movie; invalid.movieId = 12; invalid.posterAsset = "../secret.png";
    if (!check(!catalog.add(invalid, error), "asset traversal rejected")) return 1;
    invalid = movie; invalid.movieId = 12; invalid.title = std::string("Bad\xFF", 4);
    if (!check(!catalog.add(invalid, error), "invalid UTF-8 rejected")) return 1;
    invalid = movie; invalid.movieId = 12; invalid.sourceUrl.clear();
    if (!check(!catalog.add(invalid, error), "missing source rejected")) return 1;

    const auto root = std::filesystem::temp_directory_path() /
                      std::filesystem::u8path("oop_movie_điện_ảnh_test");
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);
    if (!check(!ec, "create Unicode directory")) return 1;
    const std::string path = (root / std::filesystem::u8path("phim_đẹp.txt")).u8string();
    if (!check(catalog.save(path, error), "save metadata")) return 1;
    MovieDetailsCatalog restored;
    if (!check(restored.load(path, error), "load metadata")) return 1;
    if (!check(restored.all().size() == 2 && restored.find(10) &&
               restored.find(10)->synopsis == movie.synopsis &&
               restored.find(10)->title == movie.title, "round trip UTF-8 and escapes")) return 1;
    if (!check(restored.find(10)->toV1Movie(50000).getBasePriceVnd() == 50000,
               "explicit V1 price adapter")) return 1;
    if (!check(restored.posterPath(10, root.u8string()).empty(),
               "missing poster yields fallback signal")) return 1;

    MovieDetails third = sample();
    third.movieId = 12;
    third.title = "Phim thứ ba";
    if (!check(catalog.add(third, error) && catalog.save(path, error),
               "explicit second save")) return 1;
    auto backup = std::filesystem::u8path(path);
    backup += ".bak";
    MovieDetailsCatalog previous;
    if (!check(previous.load(backup.u8string(), error) && previous.all().size() == 2,
               "previous version retained in backup")) return 1;
    MovieDetailsCatalog latest;
    if (!check(latest.load(path, error) && latest.all().size() == 3,
               "second save installs new catalog")) return 1;
    const auto directoryTarget = root / "cannot-replace-directory";
    std::filesystem::create_directory(directoryTarget, ec);
    if (!check(!catalog.save(directoryTarget.u8string(), error) &&
               std::filesystem::is_directory(directoryTarget),
               "failed save preserves existing target")) return 1;

    const auto corrupt = root / "corrupt.txt";
    { std::ofstream out(corrupt, std::ios::binary); out << "10|Bad|row\n"; }
    if (!check(!restored.load(corrupt.u8string(), error) && restored.all().size() == 2,
               "corrupt file keeps memory")) return 1;
    { std::ifstream in(std::filesystem::u8path(path), std::ios::binary);
      std::string header, first;
      std::getline(in, header); std::getline(in, first);
      std::ofstream out(corrupt, std::ios::binary);
      out << first << '\n' << first << '\n'; }
    if (!check(!restored.load(corrupt.u8string(), error) &&
               error.find("trung") != std::string::npos && restored.all().size() == 2,
               "duplicate file ID keeps memory")) return 1;
    std::filesystem::remove_all(root, ec);
    return 0;
}
