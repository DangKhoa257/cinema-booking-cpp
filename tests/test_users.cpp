#include "admin.h"
#include "customer.h"
#include "user_manager.h"
#include <iostream>
#include <sstream>
#include <type_traits>

static int failures = 0;
#define CHECK(cond)                                                      \
    do {                                                                 \
        if (!(cond)) {                                                   \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond << "\n"; \
            ++failures;                                                  \
        }                                                                \
    } while (0)

static std::string describe(const User& u) {  // da hinh qua tham chieu User
    std::ostringstream os;
    u.displayInfo(os);
    return os.str();
}

int main() {
    static_assert(std::is_abstract<User>::value, "User must be abstract");
    static_assert(!std::is_abstract<Customer>::value, "Customer concrete");
    static_assert(!std::is_abstract<Admin>::value, "Admin concrete");

    Customer c(1, "Khach A");
    Admin a(100, "Quan tri A");
    const User& uc = c;
    const User& ua = a;
    CHECK(uc.role() == "Customer");
    CHECK(ua.role() == "Admin");
    CHECK(describe(uc) != describe(ua));
    CHECK(describe(uc).find("Customer") != std::string::npos);
    CHECK(describe(ua).find("Admin") != std::string::npos);

    UserManager m;
    CHECK(m.addCustomer(Customer(1, "Khach A")));
    CHECK(m.addCustomer(Customer(2, "Khach B")));
    CHECK(m.getCustomers().size() == 2);
    const Customer* f = m.findCustomer(2);
    CHECK(f != nullptr && f->getName() == "Khach B");
    CHECK(m.findCustomer(99) == nullptr);

    CHECK(!m.addCustomer(Customer(1, "Trung ID")));
    CHECK(m.getCustomers().size() == 2);

    CHECK(!m.addCustomer(Customer(3, "")));
    CHECK(!m.addCustomer(Customer(4, "   ")));
    CHECK(!m.addCustomer(Customer(0, "ID khong")));
    CHECK(!m.addCustomer(Customer(-5, "ID am")));
    CHECK(m.getCustomers().size() == 2);

    if (failures == 0) std::cout << "All user tests passed\n";
    return failures == 0 ? 0 : 1;
}