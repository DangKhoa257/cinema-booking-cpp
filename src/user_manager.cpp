#include "user_manager.h"

bool UserManager::addCustomer(const Customer&) {
    // TODO(TV5): Kiem tra ID trung va du lieu hop le.
    return false;
}
const Customer* UserManager::findCustomer(int) const {
    // TODO(TV5): Tim khach hang theo ID.
    return nullptr;
}
const std::vector<Customer>& UserManager::getCustomers() const {
    return customers_;
}
