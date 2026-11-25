#include "ui_session.h"
#include "bank_transfer_payment.h"
#include "demo_payment.h"

UiSession::UiSession() : booking_(schedule_, rooms_, users_, tickets_) {
    movies_.addMovie(Movie(1, "Phim Demo", 90, "Giả tưởng", 50000));
    movies_.addMovie(Movie(2, "Phim Thứ Hai", 100, "Hoạt hình", 60000));
    rooms_.emplace_back(1, "Phòng Demo", std::vector<Seat>{
        Seat('A', 1), Seat('A', 2), Seat('B', 1), Seat('B', 2)});
    schedule_.addShowtime(Showtime(1, 1, 1, "2026-12-01 19:00",
                                   "2026-12-01 20:30", 50000));
    schedule_.addShowtime(Showtime(2, 2, 1, "2026-12-01 21:00",
                                   "2026-12-01 22:40", 60000));
    users_.addCustomer(Customer(1, "Khách Demo"));
}

SeatStatus UiSession::seatStatus(int showtimeId, const std::string& seatId) const {
    return booking_.getSeatStatus(showtimeId, seatId);
}

bool UiSession::addCustomer(int id, const std::string& name) {
    return users_.addCustomer(Customer(id, name));
}

int UiSession::book(int customerId, int showtimeId, const std::string& seatId,
                    bool demoPayment) {
    DemoPayment demo;
    BankTransferPayment transfer;
    Payment& method = demoPayment ? static_cast<Payment&>(demo) :
                                  static_cast<Payment&>(transfer);
    if (!booking_.bookTicket(customerId, showtimeId, seatId, method)) return 0;
    return tickets_.back().getId();
}

bool UiSession::pay(int ticketId, bool demoPayment) {
    DemoPayment demo;
    BankTransferPayment transfer;
    Payment& method = demoPayment ? static_cast<Payment&>(demo) :
                                  static_cast<Payment&>(transfer);
    return booking_.payTicket(ticketId, method);
}

bool UiSession::cancel(int ticketId) { return booking_.cancelTicket(ticketId); }
