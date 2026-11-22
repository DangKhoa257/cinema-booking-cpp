#include "bank_transfer_payment.h"
#include "demo_payment.h"
#include "ticket.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

static bool run(Payment& p, long long amount, const std::string& code) {
    return p.process(amount, code);  // da hinh qua Payment&
}

int main() {
    // Ticket hop le + chuyen trang thai
    Ticket t(1, 1, 1, "A1", 50000);
    assert(t.getStatus() == TicketStatus::Pending);
    assert(!t.setStatus(TicketStatus::Pending));   // khong doi
    assert(t.setStatus(TicketStatus::Paid));
    assert(!t.setStatus(TicketStatus::Pending));   // khong quay lui
    assert(t.setStatus(TicketStatus::Cancelled));
    assert(!t.setStatus(TicketStatus::Paid));      // khong hoi sinh
    assert(t.getStatus() == TicketStatus::Cancelled);

    Ticket t2(2, 1, 1, "A2", 50000);
    assert(t2.setStatus(TicketStatus::Cancelled)); // Pending -> Cancelled

    std::ostringstream os;
    os << t;
    assert(os.str().find("A1") != std::string::npos);
    assert(os.str().find("50000") != std::string::npos);
    assert(os.str().find("Cancelled") != std::string::npos);

    // Payment da hinh
    DemoPayment demo;
    BankTransferPayment bank;
    assert(run(demo, 50000, "BK-1"));
    assert(!run(demo, 0, "BK-1"));
    assert(!run(demo, -1, "BK-1"));
    assert(!run(demo, 50000, ""));
    assert(!run(bank, 50000, "BK-1"));  // khong tu danh dau da tra
    assert(!run(bank, 0, "BK-1"));

    std::vector<std::unique_ptr<Payment>> methods;
    methods.push_back(std::make_unique<DemoPayment>());
    methods.push_back(std::make_unique<BankTransferPayment>());
    assert(methods[0]->process(1000, "X") && !methods[1]->process(1000, "X"));

    std::cout << "test_ticket_payment OK\n";
}
