#pragma once

#include "common/types.h"
#include "cinema_room.h"
#include "schedule_manager.h"
#include "ticket.h"
#include "user_manager.h"
#include <string>
#include <vector>

class Payment;

class BookingManager {
public:
    BookingManager(const ScheduleManager& schedule,
                   std::vector<CinemaRoom>& rooms,
                   const UserManager& users,
                   std::vector<Ticket>& tickets);
    bool bookTicket(int customerId, int showtimeId,
                    const std::string& seatId, Payment& payment);
    bool cancelTicket(int ticketId);
    bool payTicket(int ticketId, Payment& payment);
    SeatStatus getSeatStatus(int showtimeId, const std::string& seatId) const;

private:
    const ScheduleManager& schedule_;
    std::vector<CinemaRoom>& rooms_;
    const UserManager& users_;
    std::vector<Ticket>& tickets_;
};
