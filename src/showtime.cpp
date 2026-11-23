#include "showtime.h"

#include <utility>

bool isValidDateTime(const std::string& s) {
    if (s.size() != 16) return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 4 || i == 7) {
            if (s[i] != '-') return false;
        } else if (i == 10) {
            if (s[i] != ' ') return false;
        } else if (i == 13) {
            if (s[i] != ':') return false;
        } else if (s[i] < '0' || s[i] > '9') {
            return false;
        }
    }
    int year = std::stoi(s.substr(0, 4));
    int month = std::stoi(s.substr(5, 2));
    int day = std::stoi(s.substr(8, 2));
    int hour = std::stoi(s.substr(11, 2));
    int minute = std::stoi(s.substr(14, 2));

    if (month < 1 || month > 12) return false;
    if (hour > 23 || minute > 59) return false;

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    int maxDay = daysInMonth[month - 1];
    if (month == 2 && leap) maxDay = 29;
    return day >= 1 && day <= maxDay;
}

Showtime::Showtime(int id, int movieId, int roomId, std::string startTime,
                   std::string endTime, long long priceVnd)
    : id_(id),
      movieId_(movieId),
      roomId_(roomId),
      startTime_(std::move(startTime)),
      endTime_(std::move(endTime)),
      priceVnd_(priceVnd) {}

int Showtime::getId() const { return id_; }
int Showtime::getMovieId() const { return movieId_; }
int Showtime::getRoomId() const { return roomId_; }
const std::string& Showtime::getStartTime() const { return startTime_; }
const std::string& Showtime::getEndTime() const { return endTime_; }
long long Showtime::getPriceVnd() const { return priceVnd_; }

bool Showtime::isValid() const {
    if (id_ <= 0 || movieId_ <= 0 || roomId_ <= 0) return false;
    if (priceVnd_ < 0) return false;
    if (!isValidDateTime(startTime_) || !isValidDateTime(endTime_)) return false;
    return startTime_ < endTime_;
}