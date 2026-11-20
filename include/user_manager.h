#pragma once

#include "customer.h"
#include <vector>

class UserManager {
public:
    bool addCustomer(const Customer& customer);
    const Customer* findCustomer(int id) const;
    const std::vector<Customer>& getCustomers() const;

private:
    std::vector<Customer> customers_;
};
