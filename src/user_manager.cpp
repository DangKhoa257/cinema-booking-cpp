#include "user_manager.h"
#include <string>

namespace {
// Hop le: ID duong, ten khong rong (khong chi gom khoang trang),
// va khong chua |, \n, \r (vi | la dau phan cach khi luu file).
bool isValid(const Customer& c) {
    if (c.getId() <= 0) return false;
    const std::string& name = c.getName();
    if (name.find_first_not_of(" \t\r\n") == std::string::npos) return false;
    if (name.find_first_of("|\n\r") != std::string::npos) return false;
    return true;
}
}  // namespace

bool UserManager::addCustomer(const Customer& customer) {
    if (!isValid(customer)) return false;
    if (findCustomer(customer.getId()) != nullptr) return false;  // chan ID trung
    customers_.push_back(customer);
    return true;
}

const Customer* UserManager::findCustomer(int id) const {
    for (const Customer& c : customers_) {
        if (c.getId() == id) return &c;
    }
    return nullptr;
}

const std::vector<Customer>& UserManager::getCustomers() const {
    return customers_;
}
