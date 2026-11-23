#include "schedule_manager.h"

bool ScheduleManager::addShowtime(const Showtime& showtime) {
    if (!showtime.isValid()) return false;
    if (findShowtime(showtime.getId()) != nullptr) return false;
    if (hasRoomConflict(showtime.getRoomId(), showtime.getStartTime(),
                        showtime.getEndTime())) {
        return false;
    }
    showtimes_.push_back(showtime);
    return true;
}

const Showtime* ScheduleManager::findShowtime(int id) const {
    for (const Showtime& s : showtimes_) {
        if (s.getId() == id) return &s;
    }
    return nullptr;
}

bool ScheduleManager::hasRoomConflict(int roomId, const std::string& start,
                                      const std::string& end) const {
    if (!isValidDateTime(start) || !isValidDateTime(end) || !(start < end)) {
        return true;
    }
    for (const Showtime& s : showtimes_) {
        if (s.getRoomId() != roomId) continue;
        if (s.getStartTime() < end && start < s.getEndTime()) return true;
    }
    return false;
}

const std::vector<Showtime>& ScheduleManager::getShowtimes() const {
    return showtimes_;
}
