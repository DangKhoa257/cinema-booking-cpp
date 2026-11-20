#include "cinema_system.h"
#include <iostream>

CinemaSystem::CinemaSystem()
    : booking_(schedule_, rooms_, users_, tickets_) {}

void CinemaSystem::run() {
    // TODO(TV1): Vong lap menu va goi cac module qua public API.
    std::cout << "Cinema Booking - Giai doan 0\n"
              << "Chuc nang dang duoc phat trien.\n";
}
