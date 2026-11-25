#include "cinema_system.h"
#include "bank_transfer_payment.h"
#include "demo_payment.h"
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
bool readInt(const std::string& prompt, int& value) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    try {
        std::size_t used = 0;
        long long number = std::stoll(line, &used);
        if (used != line.size() || number < std::numeric_limits<int>::min() ||
            number > std::numeric_limits<int>::max()) return false;
        value = static_cast<int>(number);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

const char* seatState(SeatStatus status) {
    switch (status) {
        case SeatStatus::Available: return "Trong";
        case SeatStatus::Held: return "Dang giu";
        case SeatStatus::Booked: return "Da dat";
    }
    return "Khong ro";
}

const char* ticketState(TicketStatus status) {
    switch (status) {
        case TicketStatus::Pending: return "Pending";
        case TicketStatus::Paid: return "Paid";
        case TicketStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

std::string readPath(const char* fallback) {
    std::cout << "Duong dan file (Enter = " << fallback << "): ";
    std::string path;
    std::getline(std::cin, path);
    return path.empty() ? fallback : path;
}
}  // namespace

CinemaSystem::CinemaSystem()
    : booking_(schedule_, rooms_, users_, tickets_) { seedDemoData(); }

void CinemaSystem::seedDemoData() {
    if (!movies_.addMovie(Movie(1, "Phim Demo", 90, "Gia tuong", 50000)) ||
        !movies_.addMovie(Movie(2, "Phim Thu Hai", 100, "Hoat hinh", 60000)))
        throw std::runtime_error("Khong khoi tao duoc phim demo");
    rooms_.emplace_back(1, "Phong Demo",
                        std::vector<Seat>{Seat('A', 1), Seat('A', 2),
                                          Seat('B', 1), Seat('B', 2)});
    if (!schedule_.addShowtime(Showtime(1, 1, 1, "2026-12-01 19:00",
                                        "2026-12-01 20:30", 50000)) ||
        !schedule_.addShowtime(Showtime(2, 2, 1, "2026-12-01 21:00",
                                        "2026-12-01 22:40", 60000)))
        throw std::runtime_error("Khong khoi tao duoc suat chieu demo");
    if (!users_.addCustomer(Customer(1, "Khach Demo")))
        throw std::runtime_error("Khong khoi tao duoc khach demo");
}

void CinemaSystem::showMovies() const {
    std::cout << "\nDanh sach phim:\n";
    for (const Movie& movie : movies_.getMovies()) std::cout << movie << '\n';
}

void CinemaSystem::showShowtimes() const {
    std::cout << "\nDanh sach suat chieu:\n";
    for (const Showtime& showtime : schedule_.getShowtimes()) {
        const Movie* movie = movies_.findMovie(showtime.getMovieId());
        std::cout << "#" << showtime.getId() << " | "
                  << (movie ? movie->getTitle() : "Phim khong ton tai")
                  << " | Phong " << showtime.getRoomId() << " | "
                  << showtime.getStartTime() << " - " << showtime.getEndTime()
                  << " | " << showtime.getPriceVnd() << " VND\n";
    }
}

void CinemaSystem::showRooms() const {
    std::cout << "\nPhong va ghe vat ly:\n";
    for (const CinemaRoom& room : rooms_) {
        std::cout << "Phong " << room.getId() << ": " << room.getName() << " | ";
        for (const Seat& seat : room.getSeats()) std::cout << seat.getSeatId() << ' ';
        std::cout << '\n';
    }
    int showtimeId = 0;
    if (!readInt("ID suat de xem trang thai (0=bo qua): ", showtimeId) ||
        showtimeId == 0) return;
    const Showtime* showtime = schedule_.findShowtime(showtimeId);
    if (!showtime) { std::cout << "Khong tim thay suat chieu.\n"; return; }
    for (const CinemaRoom& room : rooms_) {
        if (room.getId() != showtime->getRoomId()) continue;
        for (const Seat& seat : room.getSeats())
            std::cout << seat.getSeatId() << ": "
                      << seatState(booking_.getSeatStatus(showtimeId, seat.getSeatId())) << '\n';
        return;
    }
    std::cout << "Phong cua suat chieu khong ton tai.\n";
}

void CinemaSystem::registerCustomer() {
    int id = 0;
    if (!readInt("ID khach hang moi: ", id)) { std::cout << "ID sai.\n"; return; }
    std::cout << "Ten khach hang: ";
    std::string name;
    if (!std::getline(std::cin, name)) return;
    std::cout << (users_.addCustomer(Customer(id, name)) ?
                  "Da dang ky khach hang.\n" : "Dang ky that bai.\n");
}

void CinemaSystem::bookTicket() {
    int customerId = 0, showtimeId = 0, method = 0;
    if (!readInt("ID khach hang: ", customerId) ||
        !readInt("ID suat chieu: ", showtimeId)) {
        std::cout << "ID sai.\n"; return;
    }
    std::cout << "Ma ghe (vd A1): ";
    std::string seatId;
    if (!std::getline(std::cin, seatId)) return;
    if (!readInt("Thanh toan 1=Demo, 2=Chuyen khoan demo: ", method)) {
        std::cout << "Phuong thuc sai.\n"; return;
    }
    DemoPayment demo;
    BankTransferPayment bank;
    Payment* payment = method == 1 ? static_cast<Payment*>(&demo) :
                       method == 2 ? static_cast<Payment*>(&bank) : nullptr;
    if (!payment || !booking_.bookTicket(customerId, showtimeId, seatId, *payment)) {
        std::cout << "Dat ve that bai: kiem tra khach, suat, phong, ghe va trang thai.\n";
        return;
    }
    const Ticket& ticket = tickets_.back();
    std::cout << "Da tao ve #" << ticket.getId() << " - "
              << ticketState(ticket.getStatus()) << ".\n";
}

void CinemaSystem::showTickets() const {
    std::cout << "\nDanh sach ve:\n";
    if (tickets_.empty()) std::cout << "Chua co ve.\n";
    for (const Ticket& ticket : tickets_) std::cout << ticket << '\n';
}

void CinemaSystem::cancelTicket() {
    int id = 0;
    if (!readInt("ID ve can huy: ", id)) { std::cout << "ID sai.\n"; return; }
    std::cout << (booking_.cancelTicket(id) ? "Da huy ve.\n" :
                  "Khong the huy ve nay.\n");
}

void CinemaSystem::payTicket() {
    int id = 0, method = 0;
    if (!readInt("ID ve Pending: ", id) ||
        !readInt("Thanh toan 1=Demo, 2=Chuyen khoan demo: ", method)) {
        std::cout << "Du lieu sai.\n"; return;
    }
    DemoPayment demo;
    BankTransferPayment bank;
    Payment* payment = method == 1 ? static_cast<Payment*>(&demo) :
                       method == 2 ? static_cast<Payment*>(&bank) : nullptr;
    if (!payment) { std::cout << "Phuong thuc sai.\n"; return; }
    std::cout << (booking_.payTicket(id, *payment) ?
                  "Ve da Paid (thanh toan mo phong).\n" :
                  "Chua hoan tat thanh toan; ve hop le van Pending.\n");
}

void CinemaSystem::storageMenu() {
    int choice = 0;
    if (!readInt("1=Luu phim, 2=Tai phim, 3=Luu ve, 4=Tai ve, "
                 "5=Luu tat ca, 6=Tai tat ca: ", choice) ||
        choice < 1 || choice > 6) {
        std::cout << "Lua chon sai.\n"; return;
    }
    const std::string path = readPath(choice >= 5 ? "build/cinema_data" :
                                      choice <= 2 ? "build/movies_saved.txt" :
                                                    "build/tickets_saved.txt");
    bool ok = false;
    if (choice == 5) {
        ok = saveAll(path);
    } else if (choice == 6) {
        ok = loadAll(path);
    } else if (choice == 1) {
        ok = storage_.saveMovies(movies_.getMovies(), path);
    } else if (choice == 2) {
        std::vector<Movie> loaded;
        ok = storage_.loadMovies(loaded, path);
        if (ok) {
            MovieCatalog replacement;
            for (const Movie& movie : loaded)
                if (!replacement.addMovie(movie)) ok = false;
            for (const Showtime& showtime : schedule_.getShowtimes())
                if (!replacement.findMovie(showtime.getMovieId())) ok = false;
            if (ok) movies_ = std::move(replacement);
            else std::cout << "File thieu phim cua suat chieu hien co.\n";
        }
    } else if (choice == 3) {
        ok = storage_.saveTickets(tickets_, path);
    } else {
        std::vector<Ticket> loaded;
        ok = storage_.loadTickets(loaded, path);
        if (ok) {
            for (const Ticket& ticket : loaded) {
                const Showtime* showtime = schedule_.findShowtime(ticket.getShowtimeId());
                if (!showtime || !users_.findCustomer(ticket.getCustomerId()) ||
                    ticket.getPriceVnd() != showtime->getPriceVnd()) {
                    ok = false; break;
                }
                bool hasSeat = false;
                for (const CinemaRoom& room : rooms_)
                    if (room.getId() == showtime->getRoomId() &&
                        room.hasSeat(ticket.getSeatId())) hasSeat = true;
                if (!hasSeat) { ok = false; break; }
            }
            if (ok) tickets_ = std::move(loaded);
            else std::cout << "File ve co tham chieu hoac gia khong khop du lieu hien co.\n";
        }
    }
    if (ok) std::cout << "Thao tac file thanh cong.\n";
    else {
        std::cout << "Thao tac file that bai.";
        if (!storage_.lastError().empty()) std::cout << ' ' << storage_.lastError();
        std::cout << '\n';
    }
}

bool CinemaSystem::saveAll(const std::string& directory) {
    std::string error;
    const bool ok = dataStore_.saveAll(movies_, rooms_, schedule_, users_, tickets_,
                                       directory, error);
    if (!ok) std::cout << error << '\n';
    return ok;
}

bool CinemaSystem::loadAll(const std::string& directory) {
    std::string error;
    const bool ok = dataStore_.loadAll(movies_, rooms_, schedule_, users_, tickets_,
                                       directory, error);
    if (!ok) std::cout << error << '\n';
    return ok;
}

void CinemaSystem::run() {
    std::cout << "Cinema Booking - Demo OOP\nKhach demo: ID 1.\n";
    while (true) {
        std::cout << "\n1. Xem phim\n2. Xem suat chieu\n3. Xem phong/ghe\n"
                  << "4. Dang ky khach\n5. Dat ve\n6. Xem ve\n7. Huy ve\n"
                  << "8. Thanh toan mo phong\n9. Luu/tai du lieu\n0. Thoat\n";
        int choice = 0;
        if (!readInt("Chon: ", choice)) {
            if (std::cin.eof()) return;
            std::cout << "Lua chon sai.\n";
            continue;
        }
        switch (choice) {
            case 1: showMovies(); break;
            case 2: showShowtimes(); break;
            case 3: showRooms(); break;
            case 4: registerCustomer(); break;
            case 5: bookTicket(); break;
            case 6: showTickets(); break;
            case 7: cancelTicket(); break;
            case 8: payTicket(); break;
            case 9: storageMenu(); break;
            case 0: std::cout << "Tam biet.\n"; return;
            default: std::cout << "Lua chon sai.\n";
        }
    }
}
