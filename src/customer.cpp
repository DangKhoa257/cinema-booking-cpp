#include "customer.h"
#include <ostream>
#include <utility>

Customer::Customer(int id, std::string name) : User(id, std::move(name)) {}
std::string Customer::role() const { return "Customer"; }
void Customer::displayInfo(std::ostream& out) const {
    // TODO(TV5): Chot dinh dang hien thi thong tin demo.
    out << "Customer #" << getId() << ": " << getName();
}
