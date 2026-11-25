#pragma once

#include "movie_details.h"
#include <string>
#include <vector>

class MovieDetailsCatalog {
public:
    bool add(const MovieDetails& details, std::string& error);
    const MovieDetails* find(int movieId) const;
    const std::vector<MovieDetails>& all() const;
    std::vector<const MovieDetails*> nowShowing() const;
    std::vector<const MovieDetails*> comingSoon() const;
    std::vector<const MovieDetails*> searchTitle(const std::string& query) const;
    std::vector<const MovieDetails*> filterGenre(const std::string& genre) const;

    // Trả chuỗi rỗng khi chưa có poster hợp lệ; Qt có thể hiển thị đồ họa thay thế.
    std::string posterPath(int movieId, const std::string& assetDirectory) const;

    // Đọc cả file vào bộ nhớ tạm. Lỗi bất kỳ giữ nguyên catalog hiện tại.
    bool load(const std::string& path, std::string& error);
    // Chỉ ghi khi được gọi rõ ràng; không ghi vào năm file V1.
    bool save(const std::string& path, std::string& error) const;

private:
    std::vector<MovieDetails> movies_;
};
