#pragma once

#include "booking_manager.h"
#include "cinema_v2.h"
#include "file_storage.h"
#include "movie_details_catalog.h"

#include <QString>
#include <memory>
#include <vector>

struct SnackItem {
    QString name;
    long long price = 0;
};

struct VoucherItem {
    QString code;
    QString description;
    int percent = 0;
    long long cap = 0;
};

struct CinemaOrder {
    QString code;
    QString buyer;
    QString phone;
    QString email;
    int movieId = 0;
    int branchId = 0;
    int showtimeId = 0;
    std::vector<int> ticketIds;
    std::vector<QString> seatIds;
    std::vector<int> snackQuantities;
    QString voucher;
    long long ticketTotal = 0;
    long long snackTotal = 0;
    long long discount = 0;
};

// Lớp nối Qt với nghiệp vụ hiện có. QWidget chỉ hiển thị và gọi các hàm này.
class V2Session {
public:
    V2Session(const QString& movieFile, const QString& cinemaFile,
              const QString& storageDirectory);
    bool ready() const { return ready_; }
    const QString& error() const { return error_; }
    const QString& notice() const { return notice_; }
    const QString& storageDirectory() const { return directory_; }
    const MovieDetailsCatalog& movies() const { return movies_; }
    const CinemaV2& cinema() const { return cinema_; }
    const std::vector<Ticket>& tickets() const { return tickets_; }
    const std::vector<CinemaOrder>& orders() const { return orders_; }
    const std::vector<SnackItem>& snacks() const { return snacks_; }
    const std::vector<VoucherItem>& vouchers() const { return vouchers_; }
    SeatStatus seatStatus(int showtimeId, const std::string& seatId) const;
    long long ticketPrice(int showtimeId, const std::string& seatId) const;
    long long snackTotal(const std::vector<int>& quantities) const;
    long long discountFor(const QString& code, long long subtotal) const;
    bool checkout(int showtimeId, const std::vector<std::string>& seatIds,
                  const std::vector<int>& quantities, const QString& voucher,
                  const QString& buyer, const QString& phone, const QString& email,
                  bool payNow, QString& bookingCode);
    bool pay(const QString& bookingCode);
    bool cancel(const QString& bookingCode);
    bool save(bool allowDamagedOverwrite = false);
    bool hasDamagedSave() const { return protectedSave_; }
    bool load();
    bool dirty() const { return dirty_; }
private:
    bool loadDemo();
    bool validateTickets(const CinemaV2& cinema, const UserManager& users,
                         const std::vector<Ticket>& tickets) const;
    bool readOrders(const QString& path, std::vector<CinemaOrder>& orders) const;
    bool writeOrders(const QString& path) const;
    const CinemaOrder* findOrder(const QString& code) const;
    CinemaOrder* findOrder(const QString& code);
    void resetBooking();

    QString movieFile_;
    QString cinemaFile_;
    QString directory_;
    QString error_;
    QString notice_;
    MovieDetailsCatalog movies_;
    CinemaV2 cinema_;
    UserManager users_;
    std::vector<Ticket> tickets_;
    std::vector<CinemaOrder> orders_;
    std::vector<SnackItem> snacks_;
    std::vector<VoucherItem> vouchers_;
    std::unique_ptr<BookingManager> booking_;
    bool ready_ = false;
    bool dirty_ = false;
    bool protectedSave_ = false;
};
