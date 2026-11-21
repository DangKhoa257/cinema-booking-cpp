
#include "user_manager.h"
#include <string>

namespace {
// Hop le: ID duong, ten khong rong (khong chi gom khoang trang).
bool isValid(const Customer& c) {
    return c.getId() > 0 &&
           c.getName().find_first_not_of(" \t\r\n") != std::string::npos;
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