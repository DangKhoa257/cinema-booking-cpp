#include "ticket.h"
#include <ostream>
#include <utility>

Ticket::Ticket(int id, int customerId, int showtimeId, std::string seatId,
               long long priceVnd, TicketStatus status)
    : id_(id), customerId_(customerId), showtimeId_(showtimeId),
      seatId_(std::move(seatId)), priceVnd_(priceVnd), status_(status) {}
int Ticket::getId() const { return id_; }
int Ticket::getCustomerId() const { return customerId_; }
int Ticket::getShowtimeId() const { return showtimeId_; }
const std::string& Ticket::getSeatId() const { return seatId_; }
long long Ticket::getPriceVnd() const { return priceVnd_; }
TicketStatus Ticket::getStatus() const { return status_; }
bool Ticket::setStatus(TicketStatus) {
    // TODO(TV6): Kiem tra chuyen trang thai ve hop le.
    return false;
}
std::ostream& operator<<(std::ostream& out, const Ticket&) {
    // TODO(TV6): Dinh dang ve de hien thi.
    return out << "[Ticket: chuc nang dang duoc phat trien]";
}
