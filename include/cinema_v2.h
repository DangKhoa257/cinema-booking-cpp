#pragma once

#include "cinema_room.h"
#include "movie_details_catalog.h"
#include "schedule_manager.h"
#include <string>
#include <vector>

class BookingManager;
class Payment;

struct CinemaBranch {
    int id = 0;
    std::string name;
    std::string area;
    std::string address;
    bool active = true;
    std::vector<int> roomIds;
};

class BranchCatalog {
public:
    bool add(const CinemaBranch& branch);
    bool linkRoom(int branchId, int roomId);
    const CinemaBranch* find(int branchId) const;
    const CinemaBranch* findByRoom(int roomId) const;
    const std::vector<CinemaBranch>& all() const;
private:
    std::vector<CinemaBranch> branches_;
};

enum class SeatCategory { Standard, VIP };

struct SeatPosition {
    std::string seatId;
    char row = 'A';
    int number = 0;
    int column = 0;  // Cột hiển thị; cột trống sau lối đi không phải ghế.
    SeatCategory category = SeatCategory::Standard;
};

class RoomSeatLayout {
public:
    RoomSeatLayout(int roomId, int rowCount, int seatsPerRow,
                   int aisleAfter, int vipFromRow);
    int roomId() const;
    int rowCount() const;
    int seatsPerRow() const;
    int aisleAfter() const;
    int vipFromRow() const;
    const std::vector<SeatPosition>& positions() const;
    const SeatPosition* find(const std::string& seatId) const;
    std::vector<Seat> physicalSeats() const;
private:
    int roomId_;
    int rowCount_;
    int seatsPerRow_;
    int aisleAfter_;
    int vipFromRow_;
    std::vector<SeatPosition> positions_;
};

// Thời gian của các hàm V2 dùng Asia/Ho_Chi_Minh (UTC+07, không DST).
std::string currentVietnamTime();  // YYYY-MM-DD HH:MM

class CinemaV2 {
public:
    // Mọi tham chiếu và xung đột được kiểm tra trước khi thay dữ liệu đang dùng.
    // nowVietnam cho phép test xác định thời điểm; rỗng dùng đồng hồ hiện tại.
    bool load(const std::string& path, const MovieDetailsCatalog& movies,
              std::string& error, const std::string& nowVietnam = "");
    bool save(const std::string& path, std::string& error) const;
    bool addShowtime(const Showtime& showtime, const MovieDetailsCatalog& movies,
                     std::string& error);
    bool isBookable(int showtimeId, const std::string& nowVietnam = "") const;
    bool bookTicket(BookingManager& booking, int customerId, int showtimeId,
                    const std::string& seatId, Payment& payment,
                    const std::string& nowVietnam = "") const;
    std::vector<const Showtime*> showtimesFor(int movieId, int branchId,
                                              const std::string& date) const;
    const BranchCatalog& branches() const;
    const std::vector<CinemaRoom>& rooms() const;
    std::vector<CinemaRoom>& rooms();  // BookingManager giữ tham chiếu tới vector này.
    const RoomSeatLayout* layout(int roomId) const;
    const ScheduleManager& schedule() const;
    int cleaningMinutes() const;
private:
    BranchCatalog branches_;
    std::vector<CinemaRoom> rooms_;
    std::vector<RoomSeatLayout> layouts_;
    ScheduleManager schedule_;
    int cleaningMinutes_ = 15;
};
