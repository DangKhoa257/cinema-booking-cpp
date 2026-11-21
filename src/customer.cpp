#include "customer.h"
#include <ostream>
#include <utility>

Customer::Customer(int id, std::string name) : User(id, std::move(name)) {}
std::string Customer::role() const { return "Customer"; }
void Customer::displayInfo(std::ostream& out) const {
    out << "[Customer] ID: " << getId() << " | Name: " << getName()
        << " | Quyen: xem phim va dat ve";
}