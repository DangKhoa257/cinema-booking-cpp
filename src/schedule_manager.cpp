#include "schedule_manager.h"

bool ScheduleManager::addShowtime(const Showtime&) {
    // TODO(TV4): Kiem tra ID, phim, phong va xung dot lich.
    return false;
}
const Showtime* ScheduleManager::findShowtime(int) const {
    // TODO(TV4): Tim suat chieu theo ID.
    return nullptr;
}
bool ScheduleManager::hasRoomConflict(int, const std::string&,
                                      const std::string&) const {
    // TODO(TV4): Kiem tra hai khoang thoi gian giao nhau.
    return false;
}
const std::vector<Showtime>& ScheduleManager::getShowtimes() const {
    return showtimes_;
}
