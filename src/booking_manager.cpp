#include "booking_manager.h"
#include "payment.h"
#include <limits>

BookingManager::BookingManager(const ScheduleManager& schedule,
                               std::vector<CinemaRoom>& rooms,
                               const UserManager& users,
                               std::vector<Ticket>& tickets)
    : schedule_(schedule), rooms_(rooms), users_(users), tickets_(tickets) {}
bool BookingManager::bookTicket(int customerId, int showtimeId,
                                const std::string& seatId, Payment& payment) {
    if (customerId <= 0 || showtimeId <= 0 || seatId.empty() ||
        users_.findCustomer(customerId) == nullptr) {
        return false;
    }
    const Showtime* showtime = schedule_.findShowtime(showtimeId);
    if (showtime == nullptr || showtime->getPriceVnd() <= 0) return false;

    const CinemaRoom* room = nullptr;
    for (const CinemaRoom& candidate : rooms_) {
        if (candidate.getId() == showtime->getRoomId()) {
            room = &candidate;
            break;
        }
    }
    if (room == nullptr || !room->hasSeat(seatId) ||
        getSeatStatus(showtimeId, seatId) != SeatStatus::Available) {
        return false;
    }

    int largestId = 0;
    for (const Ticket& ticket : tickets_) {
        if (ticket.getId() > largestId) largestId = ticket.getId();
    }
    if (largestId == std::numeric_limits<int>::max()) return false;

    const int ticketId = largestId + 1;
    tickets_.emplace_back(ticketId, customerId, showtimeId, seatId,
                          showtime->getPriceVnd(), TicketStatus::Pending);
    // false cua Payment co the nghia la chuyen khoan demo van cho xac minh.
    // Ve Pending van la mot lan dat thanh cong va tiep tuc giu ghe.
    payTicket(ticketId, payment);
    return true;
}
bool BookingManager::cancelTicket(int ticketId) {
    for (Ticket& ticket : tickets_) {
        if (ticket.getId() == ticketId) {
            return ticket.setStatus(TicketStatus::Cancelled);
        }
    }
    return false;
}

bool BookingManager::payTicket(int ticketId, Payment& payment) {
    Ticket* selected = nullptr;
    for (Ticket& ticket : tickets_) {
        if (ticket.getId() == ticketId) {
            selected = &ticket;
            break;
        }
    }
    if (selected == nullptr || selected->getStatus() != TicketStatus::Pending ||
        selected->getPriceVnd() <= 0) {
        return false;
    }
    // Du lieu tai tu file co the tham chieu khach, suat hoac ghe khong ton tai.
    if (users_.findCustomer(selected->getCustomerId()) == nullptr) return false;
    const Showtime* showtime = schedule_.findShowtime(selected->getShowtimeId());
    if (showtime == nullptr || selected->getPriceVnd() != showtime->getPriceVnd())
        return false;
    const CinemaRoom* room = nullptr;
    for (const CinemaRoom& candidate : rooms_) {
        if (candidate.getId() == showtime->getRoomId()) {
            room = &candidate;
            break;
        }
    }
    if (room == nullptr || !room->hasSeat(selected->getSeatId())) return false;

    for (const Ticket& ticket : tickets_) {
        if (&ticket != selected && ticket.getShowtimeId() == selected->getShowtimeId() &&
            ticket.getSeatId() == selected->getSeatId() &&
            ticket.getStatus() != TicketStatus::Cancelled) {
            return false;
        }
    }
    const std::string code = "BK-" + std::to_string(ticketId);
    if (!payment.process(selected->getPriceVnd(), code)) return false;
    return selected->setStatus(TicketStatus::Paid);
}

SeatStatus BookingManager::getSeatStatus(int showtimeId,
                                         const std::string& seatId) const {
    // Chi xet ve cua dung suat chieu; khong dung trang thai chung cua phong.
    SeatStatus status = SeatStatus::Available;
    for (const Ticket& ticket : tickets_) {
        if (ticket.getShowtimeId() == showtimeId &&
            ticket.getSeatId() == seatId) {
            if (ticket.getStatus() == TicketStatus::Paid) {
                return SeatStatus::Booked;
            }
            if (ticket.getStatus() == TicketStatus::Pending) {
                status = SeatStatus::Held;
            }
        }
    }
    return status;
}
