#include "movie_details_catalog.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

namespace {
bool validUtf8(const std::string& value) {
    for (size_t i = 0; i < value.size();) {
        const auto first = static_cast<unsigned char>(value[i]);
        if (first == 0) return false;
        if (first < 0x80) { ++i; continue; }
        int count = 0;
        unsigned int code = 0;
        if (first >= 0xC2 && first <= 0xDF) { count = 2; code = first & 0x1F; }
        else if (first >= 0xE0 && first <= 0xEF) { count = 3; code = first & 0x0F; }
        else if (first >= 0xF0 && first <= 0xF4) { count = 4; code = first & 0x07; }
        else return false;
        if (i + count > value.size()) return false;
        for (int j = 1; j < count; ++j) {
            const auto next = static_cast<unsigned char>(value[i + j]);
            if ((next & 0xC0) != 0x80) return false;
            code = (code << 6) | (next & 0x3F);
        }
        if ((count == 3 && code < 0x800) ||
            (count == 4 && code < 0x10000) ||
            (code >= 0xD800 && code <= 0xDFFF) || code > 0x10FFFF)
            return false;
        i += count;
    }
    return true;
}

bool validDate(const std::string& value) {
    if (value.size() != 10 || value[4] != '-' || value[7] != '-') return false;
    for (size_t i = 0; i < value.size(); ++i)
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(value[i])))
            return false;
    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));
    if (year < 1900 || month < 1 || month > 12) return false;
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int limit = days[month - 1];
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
        ++limit;
    return day >= 1 && day <= limit;
}

bool validAsset(const std::string& value) {
    if (value.empty()) return true;
    if (value.find_first_of("\\:\n\r") != std::string::npos || value.front() == '/')
        return false;
    std::filesystem::path path = std::filesystem::u8path(value);
    if (path.is_absolute()) return false;
    for (const auto& part : path)
        if (part == ".." || part == "." || part.empty()) return false;
    return true;
}

bool validate(const MovieDetails& movie, std::string& error) {
    if (movie.movieId <= 0 || movie.title.find_first_not_of(" \t\r\n") == std::string::npos ||
        movie.genre.find_first_not_of(" \t\r\n") == std::string::npos ||
        movie.durationMinutes <= 0 ||
        movie.title.find_first_of("|\r\n") != std::string::npos ||
        movie.genre.find_first_of("|\r\n") != std::string::npos) {
        error = "ID, ten, the loai hoac thoi luong phim khong hop le";
        return false;
    }
    const std::string* values[] = {&movie.title, &movie.originalTitle, &movie.genre,
        &movie.ageRating, &movie.releaseDate, &movie.director, &movie.cast,
        &movie.synopsis, &movie.posterAsset, &movie.trailerUrl,
        &movie.sourceUrl, &movie.verifiedAt};
    for (const std::string* value : values)
        if (!validUtf8(*value)) { error = "Van ban khong phai UTF-8 hop le"; return false; }
    if ((!movie.releaseDate.empty() && !validDate(movie.releaseDate)) ||
        !validDate(movie.verifiedAt)) {
        error = "Ngay phai co dang YYYY-MM-DD hop le";
        return false;
    }
    if (movie.sourceUrl.rfind("https://", 0) != 0 || movie.sourceUrl.size() <= 8 ||
        movie.sourceUrl.find_first_of(" \t\r\n") != std::string::npos ||
        (!movie.trailerUrl.empty() &&
         (movie.trailerUrl.rfind("https://", 0) != 0 || movie.trailerUrl.size() <= 8 ||
          movie.trailerUrl.find_first_of(" \t\r\n") != std::string::npos))) {
        error = "Nguon va trailer (neu co) phai dung HTTPS";
        return false;
    }
    if (!validAsset(movie.posterAsset)) {
        error = "Duong dan poster phai tuong doi, khong di ra ngoai thu muc asset";
        return false;
    }
    if (movie.availability != MovieAvailability::NowShowing &&
        movie.availability != MovieAvailability::ComingSoon) {
        error = "Trang thai phim khong hop le";
        return false;
    }
    return true;
}

std::string asciiLower(std::string value) {
    for (char& c : value)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    return value;
}

std::string escape(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '\\' || c == '|') { result += '\\'; result += c; }
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else result += c;
    }
    return result;
}

bool splitEscaped(const std::string& line, std::vector<std::string>& fields) {
    std::string field;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '|') { fields.push_back(field); field.clear(); }
        else if (c == '\\') {
            if (++i == line.size()) return false;
            const char escaped = line[i];
            if (escaped == 'n') field += '\n';
            else if (escaped == 'r') field += '\r';
            else if (escaped == '|' || escaped == '\\') field += escaped;
            else return false;
        } else field += c;
    }
    fields.push_back(field);
    return true;
}

bool parsePositive(const std::string& text, int& value) {
    if (text.empty()) return false;
    for (char c : text)
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    try {
        size_t used = 0;
        value = std::stoi(text, &used);
        return used == text.size() && value > 0;
    } catch (...) { return false; }
}
}  // namespace

bool MovieDetailsCatalog::add(const MovieDetails& details, std::string& error) {
    error.clear();
    if (!validate(details, error)) return false;
    if (find(details.movieId)) { error = "ID phim trung"; return false; }
    movies_.push_back(details);
    return true;
}

const MovieDetails* MovieDetailsCatalog::find(int movieId) const {
    for (const auto& movie : movies_)
        if (movie.movieId == movieId) return &movie;
    return nullptr;
}

const std::vector<MovieDetails>& MovieDetailsCatalog::all() const { return movies_; }

std::vector<const MovieDetails*> MovieDetailsCatalog::nowShowing() const {
    std::vector<const MovieDetails*> found;
    for (const auto& movie : movies_)
        if (movie.availability == MovieAvailability::NowShowing) found.push_back(&movie);
    return found;
}

std::vector<const MovieDetails*> MovieDetailsCatalog::comingSoon() const {
    std::vector<const MovieDetails*> found;
    for (const auto& movie : movies_)
        if (movie.availability == MovieAvailability::ComingSoon) found.push_back(&movie);
    return found;
}

std::vector<const MovieDetails*> MovieDetailsCatalog::searchTitle(const std::string& query) const {
    std::vector<const MovieDetails*> found;
    const std::string needle = asciiLower(query);
    for (const auto& movie : movies_)
        if (asciiLower(movie.title).find(needle) != std::string::npos ||
            asciiLower(movie.originalTitle).find(needle) != std::string::npos)
            found.push_back(&movie);
    return found;
}

std::vector<const MovieDetails*> MovieDetailsCatalog::filterGenre(const std::string& genre) const {
    std::vector<const MovieDetails*> found;
    const std::string needle = asciiLower(genre);
    for (const auto& movie : movies_)
        if (asciiLower(movie.genre).find(needle) != std::string::npos) found.push_back(&movie);
    return found;
}

std::string MovieDetailsCatalog::posterPath(int movieId, const std::string& assetDirectory) const {
    const MovieDetails* movie = find(movieId);
    if (!movie || movie->posterAsset.empty() || !validAsset(movie->posterAsset)) return {};
    try {
        const auto root = std::filesystem::weakly_canonical(std::filesystem::u8path(assetDirectory));
        const auto path = std::filesystem::weakly_canonical(root /
                          std::filesystem::u8path(movie->posterAsset));
        const auto relative = path.lexically_relative(root);
        if (!relative.empty() && *relative.begin() != ".." &&
            std::filesystem::is_regular_file(path)) return path.u8string();
    } catch (...) { /* Missing/invalid asset is represented by an empty path. */ }
    return {};
}

bool MovieDetailsCatalog::load(const std::string& path, std::string& error) {
    error.clear();
    try {
        std::ifstream in(std::filesystem::u8path(path), std::ios::binary);
        if (!in) { error = "Khong mo duoc file metadata phim"; return false; }
        MovieDetailsCatalog next;
        std::string line;
        int number = 0;
        while (std::getline(in, line)) {
            ++number;
            if (number == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            if (line.size() > 65536) { error = "Dong metadata qua dai"; return false; }
            std::vector<std::string> c;
            if (!splitEscaped(line, c) || c.size() != 15) {
                error = "Dong " + std::to_string(number) + ": can 15 cot/escape hop le";
                return false;
            }
            MovieDetails movie;
            if (!parsePositive(c[0], movie.movieId) ||
                !parsePositive(c[4], movie.durationMinutes)) {
                error = "Dong " + std::to_string(number) + ": ID/thoi luong khong hop le";
                return false;
            }
            movie.title = c[1]; movie.originalTitle = c[2]; movie.genre = c[3];
            movie.ageRating = c[5]; movie.releaseDate = c[6]; movie.director = c[7];
            movie.cast = c[8]; movie.synopsis = c[9]; movie.posterAsset = c[10];
            movie.trailerUrl = c[11]; movie.sourceUrl = c[13]; movie.verifiedAt = c[14];
            if (c[12] == "NowShowing") movie.availability = MovieAvailability::NowShowing;
            else if (c[12] == "ComingSoon") movie.availability = MovieAvailability::ComingSoon;
            else { error = "Dong " + std::to_string(number) + ": trang thai sai"; return false; }
            std::string reason;
            if (!next.add(movie, reason)) {
                error = "Dong " + std::to_string(number) + ": " + reason;
                return false;
            }
        }
        if (in.bad()) { error = "Loi doc file metadata phim"; return false; }
        movies_ = std::move(next.movies_);
        return true;
    } catch (const std::exception& ex) {
        error = ex.what();
        return false;
    }
}

bool MovieDetailsCatalog::save(const std::string& path, std::string& error) const {
    error.clear();
    MovieDetailsCatalog checked;
    for (const auto& movie : movies_)
        if (!checked.add(movie, error)) return false;
    try {
        const auto target = std::filesystem::u8path(path);
        auto temporary = target;
        temporary += ".tmp";
        auto backup = target;
        backup += ".bak";
        {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            if (!out) { error = "Khong tao duoc file tam"; return false; }
            out << "# OOP Cinema movie details v1; 15 escaped UTF-8 fields\n";
            for (const auto& m : movies_) {
                const std::string fields[] = {std::to_string(m.movieId), m.title,
                    m.originalTitle, m.genre, std::to_string(m.durationMinutes),
                    m.ageRating, m.releaseDate, m.director, m.cast, m.synopsis,
                    m.posterAsset, m.trailerUrl,
                    m.availability == MovieAvailability::NowShowing ? "NowShowing" : "ComingSoon",
                    m.sourceUrl, m.verifiedAt};
                for (size_t i = 0; i < 15; ++i) {
                    if (i) out << '|';
                    out << escape(fields[i]);
                }
                out << '\n';
            }
            out.flush();
            if (!out) { error = "Ghi file metadata that bai"; return false; }
        }
        std::error_code ec;
        const bool existed = std::filesystem::exists(target);
        if (existed) {
            // MinGW tren Windows khong luon thay duoc backup co san bang overwrite_existing.
            std::filesystem::remove(backup, ec);
            if (ec) { error = "Khong don duoc backup cu: " + ec.message(); return false; }
            std::filesystem::copy_file(target, backup, ec);
            if (ec) { error = "Sao luu file cu that bai: " + ec.message(); return false; }
            std::filesystem::remove(target, ec);
            if (ec) { error = "Khong thay duoc file cu: " + ec.message(); return false; }
        }
        std::filesystem::rename(temporary, target, ec);
        if (ec) {
            if (existed) {
                std::error_code restore;
                std::filesystem::copy_file(backup, target,
                    std::filesystem::copy_options::overwrite_existing, restore);
            }
            error = "Khong dat duoc file moi: " + ec.message();
            return false;
        }
        return true;
    } catch (const std::exception& ex) {
        error = ex.what();
        return false;
    }
}
