#include "cinema_v2.h"
#include "booking_manager.h"
#include "payment.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {
bool validText(const std::string& text) {
    if (text.empty() || text.find_first_of("|\r\n") != std::string::npos) return false;
    for (size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i]);
        if (first == 0) return false;
        if (first < 128) { ++i; continue; }
        int count = 0;
        unsigned int code = 0;
        if (first >= 0xC2 && first <= 0xDF) { count = 2; code = first & 31; }
        else if (first >= 0xE0 && first <= 0xEF) { count = 3; code = first & 15; }
        else if (first >= 0xF0 && first <= 0xF4) { count = 4; code = first & 7; }
        else return false;
        if (i + count > text.size()) return false;
        for (int j = 1; j < count; ++j) {
            const auto next = static_cast<unsigned char>(text[i + j]);
            if ((next & 0xC0) != 0x80) return false;
            code = (code << 6) | (next & 63);
        }
        if ((count == 3 && code < 0x800) || (count == 4 && code < 0x10000) ||
            (code >= 0xD800 && code <= 0xDFFF) || code > 0x10FFFF) return false;
        i += count;
    }
    return true;
}

bool positive(const std::string& text, int& value) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; })) return false;
    try {
        size_t used = 0;
        value = std::stoi(text, &used);
        return used == text.size() && value > 0;
    } catch (...) { return false; }
}

bool nonnegative(const std::string& text, int& value) {
    if (text == "0") { value = 0; return true; }
    return positive(text, value);
}

bool priceValue(const std::string& text, long long& value) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; })) return false;
    try {
        size_t used = 0;
        value = std::stoll(text, &used);
        return used == text.size() && value > 0;
    } catch (...) { return false; }
}

std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream in(line);
    std::string field;
    while (std::getline(in, field, '|')) fields.push_back(field);
    if (!line.empty() && line.back() == '|') fields.push_back("");
    return fields;
}

// Thuật toán ngày dân sự, không phụ thuộc timezone của máy chạy.
long long daysFromCivil(int year, unsigned month, unsigned day) {
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);
    const unsigned adjustedMonth = month > 2 ? month - 3 : month + 9;
    const unsigned doy = (153 * adjustedMonth + 2) / 5 + day - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097LL + static_cast<long long>(doe) - 719468;
}

void civilFromDays(long long days, int& year, unsigned& month, unsigned& day) {
    days += 719468;
    const long long era = (days >= 0 ? days : days - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(days - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    year = static_cast<int>(yoe) + static_cast<int>(era * 400);
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    day = doy - (153 * mp + 2) / 5 + 1;
    month = mp < 10 ? mp + 3 : mp - 9;
    year += month <= 2;
}

long long minutesOf(const std::string& time) {
    const int year = std::stoi(time.substr(0, 4));
    const unsigned month = static_cast<unsigned>(std::stoi(time.substr(5, 2)));
    const unsigned day = static_cast<unsigned>(std::stoi(time.substr(8, 2)));
    const int hour = std::stoi(time.substr(11, 2));
    const int minute = std::stoi(time.substr(14, 2));
    return daysFromCivil(year, month, day) * 1440 + hour * 60 + minute;
}

std::string textOf(long long minutes) {
    long long days = minutes / 1440;
    int within = static_cast<int>(minutes % 1440);
    if (within < 0) { --days; within += 1440; }
    int year = 0;
    unsigned month = 0, day = 0;
    civilFromDays(days, year, month, day);
    std::ostringstream out;
    out << std::setfill('0') << std::setw(4) << year << '-' << std::setw(2) << month
        << '-' << std::setw(2) << day << ' ' << std::setw(2) << within / 60
        << ':' << std::setw(2) << within % 60;
    return out.str();
}

bool validClock(const std::string& clock) {
    return clock.size() == 5 && clock[2] == ':' &&
        clock[0] >= '0' && clock[0] <= '9' && clock[1] >= '0' && clock[1] <= '9' &&
        clock[3] >= '0' && clock[3] <= '9' && clock[4] >= '0' && clock[4] <= '9' &&
        std::stoi(clock.substr(0, 2)) < 24 && std::stoi(clock.substr(3, 2)) < 60;
}

bool installFile(const std::filesystem::path& target, const std::string& content,
                 std::string& error) {
    auto temporary = target; temporary += ".tmp";
    auto backup = target; backup += ".bak";
    {
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) { error = "Khong tao duoc file tam"; return false; }
        out << content;
        out.flush();
        if (!out) { error = "Ghi file tam that bai"; return false; }
    }
    std::error_code ec;
    const bool existed = std::filesystem::exists(target);
    if (existed) {
        std::filesystem::copy_file(target, backup,
            std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) { error = "Khong sao luu duoc file cu: " + ec.message(); return false; }
        std::filesystem::remove(target, ec);
        if (ec) { error = "Khong thay duoc file cu: " + ec.message(); return false; }
    }
    std::filesystem::rename(temporary, target, ec);
    if (ec) {
        if (existed) {
            std::error_code restore;
            std::filesystem::copy_file(backup, target,
                std::filesystem::copy_options::overwrite_existing, restore);
        }
        error = "Khong dat duoc file moi: " + ec.message();
        return false;
    }
    return true;
}
}  // namespace

bool BranchCatalog::add(const CinemaBranch& branch) {
    if (branch.id <= 0 || !validText(branch.name) || !validText(branch.area) ||
        !validText(branch.address) || !branch.roomIds.empty() || find(branch.id)) return false;
    branches_.push_back(branch);
    return true;
}

bool BranchCatalog::linkRoom(int branchId, int roomId) {
    if (roomId <= 0 || findByRoom(roomId)) return false;
    for (CinemaBranch& branch : branches_)
        if (branch.id == branchId) { branch.roomIds.push_back(roomId); return true; }
    return false;
}

const CinemaBranch* BranchCatalog::find(int branchId) const {
    for (const auto& branch : branches_) if (branch.id == branchId) return &branch;
    return nullptr;
}

const CinemaBranch* BranchCatalog::findByRoom(int roomId) const {
    for (const auto& branch : branches_)
        for (int id : branch.roomIds) if (id == roomId) return &branch;
    return nullptr;
}

const std::vector<CinemaBranch>& BranchCatalog::all() const { return branches_; }

RoomSeatLayout::RoomSeatLayout(int roomId, int rowCount, int seatsPerRow,
                               int aisleAfter, int vipFromRow)
    : roomId_(roomId), rowCount_(rowCount), seatsPerRow_(seatsPerRow),
      aisleAfter_(aisleAfter), vipFromRow_(vipFromRow) {
    if (roomId <= 0 || rowCount < 1 || rowCount > 26 || seatsPerRow < 2 ||
        seatsPerRow > 30 || aisleAfter < 1 || aisleAfter >= seatsPerRow ||
        vipFromRow < 1 || vipFromRow > rowCount ||
        rowCount * seatsPerRow < 80 || rowCount * seatsPerRow > 120)
        throw std::invalid_argument("Kich thuoc layout, loi di hoac hang VIP khong hop le");
    for (int row = 1; row <= rowCount; ++row)
        for (int number = 1; number <= seatsPerRow; ++number) {
            Seat seat(static_cast<char>('A' + row - 1), number);
            positions_.push_back({seat.getSeatId(), seat.getRow(), number,
                number + (number > aisleAfter ? 1 : 0),
                row >= vipFromRow ? SeatCategory::VIP : SeatCategory::Standard});
        }
}

int RoomSeatLayout::roomId() const { return roomId_; }
int RoomSeatLayout::rowCount() const { return rowCount_; }
int RoomSeatLayout::seatsPerRow() const { return seatsPerRow_; }
int RoomSeatLayout::aisleAfter() const { return aisleAfter_; }
int RoomSeatLayout::vipFromRow() const { return vipFromRow_; }
const std::vector<SeatPosition>& RoomSeatLayout::positions() const { return positions_; }

const SeatPosition* RoomSeatLayout::find(const std::string& seatId) const {
    for (const auto& seat : positions_) if (seat.seatId == seatId) return &seat;
    return nullptr;
}

std::vector<Seat> RoomSeatLayout::physicalSeats() const {
    std::vector<Seat> seats;
    for (const auto& seat : positions_) seats.emplace_back(seat.row, seat.number);
    return seats;
}

std::string currentVietnamTime() {
    const auto now = std::chrono::system_clock::now() + std::chrono::hours(7);
    const std::time_t instant = std::chrono::system_clock::to_time_t(now);
    std::tm parts{};
#ifdef _WIN32
    gmtime_s(&parts, &instant);
#else
    gmtime_r(&instant, &parts);
#endif
    std::ostringstream out;
    out << std::put_time(&parts, "%Y-%m-%d %H:%M");
    return out.str();
}

bool CinemaV2::addShowtime(const Showtime& showtime,
                            const MovieDetailsCatalog& movies, std::string& error) {
    error.clear();
    const MovieDetails* movie = movies.find(showtime.getMovieId());
    const CinemaBranch* branch = branches_.findByRoom(showtime.getRoomId());
    if (!showtime.isValid() || !movie || !branch ||
        showtime.getPriceVnd() <= 0) {
        error = "Suat chieu tham chieu phim/phong khong ton tai hoac khong hop le";
        return false;
    }
    const long long start = minutesOf(showtime.getStartTime());
    const long long end = minutesOf(showtime.getEndTime());
    if (end - start != movie->durationMinutes) {
        error = "Thoi gian chieu khong khop thoi luong phim";
        return false;
    }
    for (const Showtime& existing : schedule_.getShowtimes()) {
        if (existing.getRoomId() != showtime.getRoomId()) continue;
        const long long oldStart = minutesOf(existing.getStartTime());
        const long long oldEnd = minutesOf(existing.getEndTime());
        if (oldStart < end + cleaningMinutes_ && start < oldEnd + cleaningMinutes_) {
            error = "Xung dot lich chieu va thoi gian ve sinh cung phong";
            return false;
        }
    }
    if (!schedule_.addShowtime(showtime)) {
        error = "Trung ID suat chieu hoac xung dot phong";
        return false;
    }
    return true;
}

bool CinemaV2::load(const std::string& path, const MovieDetailsCatalog& movies,
                    std::string& error, const std::string& nowVietnam) {
    error.clear();
    const std::string now = nowVietnam.empty() ? currentVietnamTime() : nowVietnam;
    if (!isValidDateTime(now)) { error = "Thoi gian Viet Nam khong hop le"; return false; }
    try {
        std::ifstream in(std::filesystem::u8path(path), std::ios::binary);
        if (!in) { error = "Khong mo duoc file cum rap V2"; return false; }
        CinemaV2 next;
        std::string line;
        int lineNo = 0;
        bool hasConfig = false;
        while (std::getline(in, line)) {
            ++lineNo;
            if (lineNo == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            if (line.size() > 65536) { error = "Dong qua dai"; return false; }
            const auto c = split(line);
            if (!hasConfig && (c.empty() || c[0] != "C")) {
                error = "Dong " + std::to_string(lineNo) + ": can cau hinh C dau tien";
                return false;
            }
            int id = 0;
            bool good = false;
            if (c.size() == 2 && c[0] == "C" && !hasConfig &&
                nonnegative(c[1], id) && id <= 180) {
                next.cleaningMinutes_ = id;
                hasConfig = true;
                good = true;
            } else if (c.size() == 6 && c[0] == "B" && positive(c[1], id) &&
                       (c[5] == "1" || c[5] == "0")) {
                good = next.branches_.add({id, c[2], c[3], c[4], c[5] == "1", {}});
            } else if (c.size() == 8 && c[0] == "R" && positive(c[1], id)) {
                int branchId = 0, rows = 0, seats = 0, aisle = 0, vip = 0;
                if (positive(c[2], branchId) && positive(c[4], rows) &&
                    positive(c[5], seats) && positive(c[6], aisle) &&
                    positive(c[7], vip) && validText(c[3]) &&
                    next.branches_.find(branchId)) {
                    RoomSeatLayout layout(id, rows, seats, aisle, vip);
                    good = next.branches_.linkRoom(branchId, id);
                    if (good) {
                        next.rooms_.emplace_back(id, c[3], layout.physicalSeats());
                        next.layouts_.push_back(std::move(layout));
                    }
                }
            } else if (c.size() == 7 && (c[0] == "T" || c[0] == "S") &&
                       positive(c[1], id)) {
                int movieId = 0, roomId = 0;
                long long price = 0;
                if (positive(c[2], movieId) && positive(c[3], roomId) &&
                    priceValue(c[6], price)) {
                    std::string start, end;
                    if (c[0] == "T") {
                        int offset = 0;
                        const MovieDetails* movie = movies.find(movieId);
                        if (positive(c[4], offset) && offset <= 30 &&
                            validClock(c[5]) && movie) {
                            const long long day = minutesOf(now.substr(0, 10) + " 00:00") / 1440;
                            start = textOf((day + offset) * 1440 +
                                std::stoi(c[5].substr(0, 2)) * 60 + std::stoi(c[5].substr(3, 2)));
                            end = textOf(minutesOf(start) + movie->durationMinutes);
                        }
                    } else { start = c[4]; end = c[5]; }
                    if (!start.empty())
                        good = next.addShowtime(Showtime(id, movieId, roomId, start, end, price),
                                                movies, error);
                }
            }
            if (!good) {
                if (error.empty()) error = "Du lieu V2 sai, trung ID hoac tham chieu loi";
                error = "Dong " + std::to_string(lineNo) + ": " + error;
                return false;
            }
        }
        if (in.bad()) { error = "Loi doc file cum rap V2"; return false; }
        if (!hasConfig || next.branches_.all().empty() || next.rooms_.empty() ||
            next.schedule_.getShowtimes().empty()) {
            error = "Ban luu V2 thieu cau hinh, cum rap, phong hoac suat";
            return false;
        }
        *this = std::move(next);
        return true;
    } catch (const std::exception& ex) {
        error = std::string("File V2 khong hop le: ") + ex.what();
        return false;
    }
}

bool CinemaV2::save(const std::string& path, std::string& error) const {
    error.clear();
    try {
        std::ostringstream out;
        out << "# OOP Cinema V2; thoi gian Asia/Ho_Chi_Minh\nC|" << cleaningMinutes_ << '\n';
        for (const auto& branch : branches_.all())
            out << "B|" << branch.id << '|' << branch.name << '|' << branch.area
                << '|' << branch.address << '|' << (branch.active ? 1 : 0) << '\n';
        for (const auto& room : rooms_) {
            const auto* branch = branches_.findByRoom(room.getId());
            const auto* seatLayout = layout(room.getId());
            if (!branch || !seatLayout || !validText(room.getName()) ||
                room.getSeats().size() != seatLayout->positions().size()) {
                error = "Phong/layout khong hop le";
                return false;
            }
            for (const Seat& seat : room.getSeats())
                if (!seatLayout->find(seat.getSeatId())) {
                    error = "Ghe vat ly khong khop layout";
                    return false;
                }
            out << "R|" << room.getId() << '|' << branch->id << '|' << room.getName()
                << '|' << seatLayout->rowCount() << '|' << seatLayout->seatsPerRow()
                << '|' << seatLayout->aisleAfter() << '|' << seatLayout->vipFromRow() << '\n';
        }
        for (const auto& show : schedule_.getShowtimes())
            out << "S|" << show.getId() << '|' << show.getMovieId() << '|'
                << show.getRoomId() << '|' << show.getStartTime() << '|'
                << show.getEndTime() << '|' << show.getPriceVnd() << '\n';
        return installFile(std::filesystem::u8path(path), out.str(), error);
    } catch (const std::exception& ex) {
        error = ex.what();
        return false;
    }
}

bool CinemaV2::isBookable(int showtimeId, const std::string& nowVietnam) const {
    const auto* show = schedule_.findShowtime(showtimeId);
    const std::string now = nowVietnam.empty() ? currentVietnamTime() : nowVietnam;
    const auto* branch = show ? branches_.findByRoom(show->getRoomId()) : nullptr;
    return show && branch && branch->active && isValidDateTime(now) &&
           now < show->getStartTime();
}

bool CinemaV2::bookTicket(BookingManager& booking, int customerId, int showtimeId,
                           const std::string& seatId, Payment& payment,
                           const std::string& nowVietnam) const {
    return isBookable(showtimeId, nowVietnam) &&
           booking.bookTicket(customerId, showtimeId, seatId, payment);
}

std::vector<const Showtime*> CinemaV2::showtimesFor(int movieId, int branchId,
                                                     const std::string& date) const {
    std::vector<const Showtime*> result;
    if (date.size() != 10 || !isValidDateTime(date + " 00:00")) return result;
    for (const auto& show : schedule_.getShowtimes()) {
        const auto* branch = branches_.findByRoom(show.getRoomId());
        if (show.getMovieId() == movieId && branch && branch->id == branchId &&
            show.getStartTime().compare(0, 10, date) == 0)
            result.push_back(&show);
    }
    return result;
}

const BranchCatalog& CinemaV2::branches() const { return branches_; }
const std::vector<CinemaRoom>& CinemaV2::rooms() const { return rooms_; }
std::vector<CinemaRoom>& CinemaV2::rooms() { return rooms_; }
const RoomSeatLayout* CinemaV2::layout(int roomId) const {
    for (const auto& item : layouts_) if (item.roomId() == roomId) return &item;
    return nullptr;
}
const ScheduleManager& CinemaV2::schedule() const { return schedule_; }
int CinemaV2::cleaningMinutes() const { return cleaningMinutes_; }
