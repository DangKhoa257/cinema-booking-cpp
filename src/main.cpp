#include "cinema_system.h"
#include <exception>
#include <iostream>

int main() {
    try {
        CinemaSystem system;
        system.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Loi khoi dong: " << error.what() << '\n';
        return 1;
    }
}
