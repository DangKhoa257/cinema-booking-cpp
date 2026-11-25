#include "ticket.h"
#include <ostream>
#include <utility>

namespace {
const char* statusName(TicketStatus s) {
    switch (s) {
        case TicketStatus::Pending:   return "Pending";
        case TicketStatus::Paid:      return "Paid";
        case TicketStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}
}  // namespace

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

// Quy tac: Pending -> Paid | Cancelled; Paid -> Cancelled.
// Cancelled la trang thai cuoi (khong hoi sinh ve). Giu nguyen trang thai
// hoac chuyen nguoc (Paid -> Pending) deu bi tu choi.
bool Ticket::setStatus(TicketStatus status) {
    bool ok = false;
    switch (status_) {
        case TicketStatus::Pending:
            ok = (status == TicketStatus::Paid || status == TicketStatus::Cancelled);
            break;
        case TicketStatus::Paid:
            ok = (status == TicketStatus::Cancelled);
            break;
        case TicketStatus::Cancelled:
            ok = false;
            break;
    }
    if (ok) status_ = status;
    return ok;
}

std::ostream& operator<<(std::ostream& out, const Ticket& t) {
    return out << "Ve #" << t.getId() << " | Khach: " << t.getCustomerId()
               << " | Suat: " << t.getShowtimeId() << " | Ghe: " << t.getSeatId()
               << " | Gia: " << t.getPriceVnd() << " VND"
               << " | Trang thai: " << statusName(t.getStatus());
}
