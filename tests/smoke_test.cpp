#include "admin.h"
#include "bank_transfer_payment.h"
#include "booking_manager.h"
#include "cinema_system.h"
#include "customer.h"
#include "demo_payment.h"
#include "movie.h"
#include "seat.h"
#include "ticket.h"
#include <cassert>
#include <sstream>
#include <type_traits>

int main() {
    static_assert(std::is_abstract<User>::value, "User must be abstract");
    static_assert(std::is_abstract<Payment>::value, "Payment must be abstract");
    static_assert(std::has_virtual_destructor<User>::value, "User needs virtual destructor");
    static_assert(std::has_virtual_destructor<Payment>::value, "Payment needs virtual destructor");

    Movie movie(1, "Demo", 90, "Demo", 50000);
    assert(movie.getId() == 1 && movie.getBasePriceVnd() == 50000);
    Seat seat('A', 1);
    assert(seat.getSeatId() == "A1");
    Ticket ticket(1, 1, 1, "A1", 50000);
    assert(ticket.getStatus() == TicketStatus::Pending);
    std::vector<Ticket> tickets;
    tickets.push_back(ticket);
    tickets.emplace_back(2, 1, 2, "A1", 50000, TicketStatus::Paid);
    ScheduleManager schedule;
    std::vector<CinemaRoom> rooms;
    UserManager users;
    BookingManager booking(schedule, rooms, users, tickets);
    assert(booking.getSeatStatus(1, "A1") == SeatStatus::Held);
    assert(booking.getSeatStatus(2, "A1") == SeatStatus::Booked);
    assert(booking.getSeatStatus(3, "A1") == SeatStatus::Available);
    assert(booking.getSeatStatus(1, "A2") == SeatStatus::Available);
    Customer customer(1, "Khach demo");
    Admin admin(2, "Quan tri demo");
    assert(customer.role() == "Customer" && admin.role() == "Admin");
    User& user = customer;
    assert(user.role() == "Customer");
    std::ostringstream out;
    user.displayInfo(out);
    assert(!out.str().empty());
    out << movie << ticket;
    assert(!out.str().empty());

    DemoPayment demo;
    BankTransferPayment bank;
    Payment& demoMethod = demo;
    Payment& bankMethod = bank;
    assert(!demoMethod.process(50000, "DEMO-1")); // Skeleton: chua thanh toan.
    assert(!bankMethod.process(50000, "DEMO-1")); // Khong tu xac nhan chuyen khoan.
    CinemaSystem system;
    system.run();
}
