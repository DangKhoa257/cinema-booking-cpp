#include "demo_payment.h"
#include <iostream>

// Chi mo phong: khong co tien that nao di chuyen.
bool DemoPayment::process(long long amountVnd, const std::string& bookingCode) {
    if (amountVnd <= 0 || bookingCode.empty()) {
        std::cout << "[DEMO] Tu choi: so tien phai > 0 va ma dat ve khong rong.\n";
        return false;
    }
    std::cout << "[DEMO] Thanh toan MO PHONG thanh cong " << amountVnd
              << " VND cho ma " << bookingCode << " (khong co giao dich that).\n";
    return true;
}
