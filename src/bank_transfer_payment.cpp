#include "bank_transfer_payment.h"
#include <iostream>

// Chi phat huong dan DEMO va tra false de ve giu Pending.
// Khong co so tai khoan/QR that; bam nut hay quet ma khong chung minh da chuyen tien.
bool BankTransferPayment::process(long long amountVnd, const std::string& bookingCode) {
    if (amountVnd <= 0 || bookingCode.empty()) {
        std::cout << "[DEMO] Tu choi: so tien phai > 0 va ma dat ve khong rong.\n";
        return false;
    }
    std::cout << "[DEMO] Huong dan chuyen khoan thu nghiem (KHONG phai giao dich that)\n"
              << "  So tien   : " << amountVnd << " VND\n"
              << "  Noi dung  : DEMO " << bookingCode << "\n"
              << "  Trang thai: CHO XAC NHAN thu cong - ve van o Pending.\n";
    return false;
}
