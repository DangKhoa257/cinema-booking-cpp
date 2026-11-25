#pragma once

#include "booking_manager.h"
#include "cinema_data_store.h"
#include "movie_catalog.h"

// Du lieu cua mot phien GUI. Nghiep vu dat/huy/tra tien van do BookingManager xu ly.
class UiSession {
public:
    UiSession();
    const MovieCatalog& movies() const { return movies_; }
    const ScheduleManager& schedule() const { return schedule_; }
    const std::vector<CinemaRoom>& rooms() const { return rooms_; }
    const UserManager& users() const { return users_; }
    const std::vector<Ticket>& tickets() const { return tickets_; }
    SeatStatus seatStatus(int showtimeId, const std::string& seatId) const;
    bool addCustomer(int id, const std::string& name);
    int book(int customerId, int showtimeId, const std::string& seatId,
             bool demoPayment);
    bool pay(int ticketId, bool demoPayment);
    bool cancel(int ticketId);
    bool save(const std::string& directory);
    bool load(const std::string& directory);
    bool dirty() const { return dirty_; }
    const std::string& lastError() const { return lastError_; }

private:
    MovieCatalog movies_;
    std::vector<CinemaRoom> rooms_;
    ScheduleManager schedule_;
    UserManager users_;
    std::vector<Ticket> tickets_;
    CinemaDataStore dataStore_;
    BookingManager booking_;
    bool dirty_ = true;  // du lieu demo chua duoc luu
    std::string lastError_;
};
