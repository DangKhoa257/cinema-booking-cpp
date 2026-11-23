#pragma once

#include <string>
#include <vector>

#include "showtime.h"

class ScheduleManager {
public:
    bool addShowtime(const Showtime& showtime);
    const Showtime* findShowtime(int id) const;
    bool hasRoomConflict(int roomId, const std::string& start,
                         const std::string& end) const;
    const std::vector<Showtime>& getShowtimes() const;

private:
    std::vector<Showtime> showtimes_;
};