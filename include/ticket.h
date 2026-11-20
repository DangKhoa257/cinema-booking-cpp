#pragma once

#include "common/types.h"
#include <iosfwd>
#include <string>

class Ticket {
public:
    Ticket(int id, int customerId, int showtimeId, std::string seatId,
           long long priceVnd, TicketStatus status = TicketStatus::Pending);
    int getId() const;
    int getCustomerId() const;
    int getShowtimeId() const;
    const std::string& getSeatId() const;
    long long getPriceVnd() const;
    TicketStatus getStatus() const;
    bool setStatus(TicketStatus status);

private:
    int id_;
    int customerId_;
    int showtimeId_;
    std::string seatId_;
    long long priceVnd_;
    TicketStatus status_;
};

std::ostream& operator<<(std::ostream& out, const Ticket& ticket);
