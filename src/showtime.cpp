#include "showtime.h"
#include <utility>

Showtime::Showtime(int id, int movieId, int roomId, std::string startTime,
                   std::string endTime, long long priceVnd)
    : id_(id), movieId_(movieId), roomId_(roomId),
      startTime_(std::move(startTime)), endTime_(std::move(endTime)),
      priceVnd_(priceVnd) {}
int Showtime::getId() const { return id_; }
int Showtime::getMovieId() const { return movieId_; }
int Showtime::getRoomId() const { return roomId_; }
const std::string& Showtime::getStartTime() const { return startTime_; }
const std::string& Showtime::getEndTime() const { return endTime_; }
long long Showtime::getPriceVnd() const { return priceVnd_; }
