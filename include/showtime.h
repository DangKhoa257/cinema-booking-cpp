#pragma once

#include <string>

class Showtime {
public:
    Showtime(int id, int movieId, int roomId, std::string startTime,
             std::string endTime, long long priceVnd);
    int getId() const;
    int getMovieId() const;
    int getRoomId() const;
    const std::string& getStartTime() const;
    const std::string& getEndTime() const;
    long long getPriceVnd() const;

private:
    int id_;
    int movieId_;
    int roomId_;
    std::string startTime_;
    std::string endTime_;
    long long priceVnd_;
};
