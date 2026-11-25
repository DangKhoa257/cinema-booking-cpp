#pragma once

#include "movie.h"
#include <string>

enum class MovieAvailability { NowShowing, ComingSoon };

// Metadata biên tập của phim, độc lập với giá vé và lịch chiếu OOP Cinema.
// Chuỗi rỗng ở các trường tùy chọn có nghĩa là nguồn chưa xác minh thông tin đó.
struct MovieDetails {
    int movieId = 0;
    std::string title;
    std::string originalTitle;
    std::string genre;
    int durationMinutes = 0;
    std::string ageRating;
    std::string releaseDate;       // YYYY-MM-DD, có thể rỗng
    std::string director;
    std::string cast;
    std::string synopsis;
    std::string posterAsset;       // đường dẫn tương đối trong thư mục asset
    std::string trailerUrl;
    MovieAvailability availability = MovieAvailability::ComingSoon;
    std::string sourceUrl;
    std::string verifiedAt;        // YYYY-MM-DD

    // Giá là dữ liệu nội bộ OOP Cinema do caller truyền rõ ràng.
    Movie toV1Movie(long long basePriceVnd) const;
};
