#pragma once

#include "booking_manager.h"
#include "file_storage.h"
#include "movie_catalog.h"
#include <vector>

class CinemaSystem {
public:
    CinemaSystem();
    void run();

private:
    void seedDemoData();
    void showMovies() const;
    void showShowtimes() const;
    void showRooms() const;
    void registerCustomer();
    void bookTicket();
    void showTickets() const;
    void cancelTicket();
    void payTicket();
    void storageMenu();

    MovieCatalog movies_;
    std::vector<CinemaRoom> rooms_;
    ScheduleManager schedule_;
    UserManager users_;
    std::vector<Ticket> tickets_;
    FileStorage storage_;
    BookingManager booking_;
};
