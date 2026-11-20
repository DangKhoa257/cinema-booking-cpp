#pragma once

#include <iosfwd>
#include <string>

class Movie {
public:
    Movie(int id, std::string title, int durationMinutes, std::string genre,
          long long basePriceVnd);
    int getId() const;
    const std::string& getTitle() const;
    int getDurationMinutes() const;
    const std::string& getGenre() const;
    long long getBasePriceVnd() const;

private:
    int id_;
    std::string title_;
    int durationMinutes_;
    std::string genre_;
    long long basePriceVnd_;
};

std::ostream& operator<<(std::ostream& out, const Movie& movie);
