#pragma once

#include "file_storage.h"
#include "movie_catalog.h"
#include "schedule_manager.h"
#include "user_manager.h"

// TV1: dieu phoi nam file va kiem tra tham chieu cheo cho console va Qt.
// loadAll chi thay du lieu dang dung khi toan bo ban luu hop le.
class CinemaDataStore {
public:
    bool saveAll(const MovieCatalog& movies, const std::vector<CinemaRoom>& rooms,
                 const ScheduleManager& schedule, const UserManager& users,
                 const std::vector<Ticket>& tickets, const std::string& directory,
                 std::string& error);
    bool loadAll(MovieCatalog& movies, std::vector<CinemaRoom>& rooms,
                 ScheduleManager& schedule, UserManager& users,
                 std::vector<Ticket>& tickets, const std::string& directory,
                 std::string& error);

private:
    FileStorage storage_;
};
