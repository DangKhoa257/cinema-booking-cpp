#include "admin.h"
#include <ostream>
#include <utility>

Admin::Admin(int id, std::string name) : User(id, std::move(name)) {}
std::string Admin::role() const { return "Admin"; }
void Admin::displayInfo(std::ostream& out) const {
    out << "[Admin] ID: " << getId() << " | Name: " << getName()
        << " | Quyen: quan ly phim va lich chieu";
}