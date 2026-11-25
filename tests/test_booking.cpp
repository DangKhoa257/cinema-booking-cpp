#include "bank_transfer_payment.h"
#include "booking_manager.h"
#include "demo_payment.h"
#include <iostream>
#include <limits>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

class InspectPayment : public Payment {
public:
    explicit InspectPayment(const std::vector<Ticket>& tickets) : tickets_(tickets) {}
    bool process(long long amountVnd, const std::string& code) override {
        sawPending_ = !tickets_.empty() &&
                      tickets_.back().getStatus() == TicketStatus::Pending &&
                      amountVnd > 0 && code == "BK-5";
        return false;
    }
    bool sawPending() const { return sawPending_; }
private:
    const std::vector<Ticket>& tickets_;
    bool sawPending_ = false;
};
}  // namespace

int main() {
    ScheduleManager schedule;
    check(schedule.addShowtime(Showtime(1, 1, 1, "2026-12-01 19:00",
                                        "2026-12-01 20:30", 50000)), "showtime 1");
    check(schedule.addShowtime(Showtime(2, 1, 1, "2026-12-01 21:00",
                                        "2026-12-01 22:30", 50000)), "showtime 2");
    check(schedule.addShowtime(Showtime(3, 1, 2, "2026-12-02 19:00",
                                        "2026-12-02 20:30", 50000)), "showtime without room");
    check(schedule.addShowtime(Showtime(4, 1, 1, "2026-12-02 21:00",
                                        "2026-12-02 22:30", 0)), "free showtime");
    std::vector<CinemaRoom> rooms;
    rooms.emplace_back(1, "Demo", std::vector<Seat>{Seat('A', 1), Seat('A', 2)});
    UserManager users;
    check(users.addCustomer(Customer(1, "Khach Demo")), "customer exists");
    std::vector<Ticket> tickets;
    BookingManager booking(schedule, rooms, users, tickets);
    DemoPayment demo;
    BankTransferPayment bank;

    check(!booking.bookTicket(9, 1, "A1", demo), "reject unknown customer");
    check(!booking.bookTicket(1, 9, "A1", demo), "reject unknown showtime");
    check(!booking.bookTicket(1, 3, "A1", demo), "reject missing room");
    check(!booking.bookTicket(1, 1, "Z9", demo), "reject missing seat");
    check(!booking.bookTicket(1, 4, "A1", demo), "reject nonpositive payment amount");
    check(tickets.empty(), "invalid bookings do not create tickets");

    check(booking.bookTicket(1, 1, "A1", demo), "book with demo payment");
    check(tickets.size() == 1 && tickets[0].getId() == 1 &&
          tickets[0].getStatus() == TicketStatus::Paid, "demo ticket paid");
    check(booking.getSeatStatus(1, "A1") == SeatStatus::Booked, "paid seat booked");
    check(!booking.bookTicket(1, 1, "A1", demo), "reject duplicate seat in same showtime");
    check(booking.bookTicket(1, 2, "A1", bank), "same physical seat in another showtime");
    check(tickets.size() == 2 && tickets[1].getId() == 2 &&
          tickets[1].getStatus() == TicketStatus::Pending, "bank transfer stays pending");
    check(booking.getSeatStatus(2, "A1") == SeatStatus::Held, "pending seat held");
    check(!booking.bookTicket(1, 2, "A1", demo), "reject duplicate held seat");
    check(!booking.payTicket(2, bank) && tickets[1].getStatus() == TicketStatus::Pending,
          "bank retry cannot confirm payment");
    check(booking.payTicket(2, demo) && tickets[1].getStatus() == TicketStatus::Paid,
          "explicit demo payment completes pending ticket");
    check(!booking.payTicket(2, demo), "cannot pay already paid ticket");

    check(booking.cancelTicket(1), "cancel paid ticket");
    check(booking.getSeatStatus(1, "A1") == SeatStatus::Available,
          "cancel frees only this showtime");
    check(booking.getSeatStatus(2, "A1") == SeatStatus::Booked,
          "other showtime stays booked");
    check(!booking.cancelTicket(1), "cannot cancel twice");
    check(!booking.payTicket(1, demo), "cannot pay cancelled ticket");
    check(booking.bookTicket(1, 1, "A1", demo) && tickets.back().getId() == 3,
          "new booking uses unique increasing ID");

    check(booking.bookTicket(1, 1, "A2", bank), "second seat can be held");
    check(tickets.back().getId() == 4 && booking.cancelTicket(4),
          "cancel pending ticket");
    check(booking.getSeatStatus(1, "A2") == SeatStatus::Available,
          "cancel pending frees seat");

    InspectPayment inspect(tickets);
    check(booking.bookTicket(1, 2, "A2", inspect), "failed payment leaves pending booking");
    check(inspect.sawPending() && tickets.back().getId() == 5 &&
          tickets.back().getStatus() == TicketStatus::Pending,
          "pending ticket exists before payment process");
    check(!booking.cancelTicket(999) && !booking.payTicket(999, demo),
          "unknown ticket IDs rejected");

    check(booking.cancelTicket(5), "cancel inspection ticket");
    tickets.emplace_back(6, 1, 2, "A2", 1, TicketStatus::Pending);
    check(!booking.payTicket(6, demo) &&
          tickets.back().getStatus() == TicketStatus::Pending,
          "reject loaded ticket with price different from showtime");

    tickets.emplace_back(std::numeric_limits<int>::max(), 1, 1, "A2", 50000,
                         TicketStatus::Cancelled);
    check(!booking.bookTicket(1, 1, "A2", demo), "ticket ID overflow rejected");

    return failures == 0 ? 0 : 1;
}
