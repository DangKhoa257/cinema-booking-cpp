#include "booking_manager.h"
#include "payment.h"

BookingManager::BookingManager(const ScheduleManager& schedule,
                               std::vector<CinemaRoom>& rooms,
                               const UserManager& users,
                               std::vector<Ticket>& tickets)
    : schedule_(schedule), rooms_(rooms), users_(users), tickets_(tickets) {}
bool BookingManager::bookTicket(int, int, const std::string&, Payment&) {
    // TODO(TV1): Validate IDs va ghe; tu choi neu getSeatStatus != Available.
    // Tao ve Pending; chi danh dau Paid khi thanh toan demo thanh cong.
    return false;
}
bool BookingManager::cancelTicket(int) {
    // TODO(TV1): Chuyen ve sang Cancelled; trang thai ghe duoc suy ra tu ve.
    return false;
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
