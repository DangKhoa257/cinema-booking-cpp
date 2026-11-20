#include "admin.h"
#include <ostream>
#include <utility>

Admin::Admin(int id, std::string name) : User(id, std::move(name)) {}
std::string Admin::role() const { return "Admin"; }
void Admin::displayInfo(std::ostream& out) const {
    // TODO(TV5): Chot dinh dang hien thi thong tin demo.
    out << "Admin #" << getId() << ": " << getName();
}
