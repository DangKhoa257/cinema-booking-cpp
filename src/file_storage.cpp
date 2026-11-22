#include "file_storage.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

namespace {

bool hasForbiddenChar(const std::string& s) {
    return s.find('|') != std::string::npos || s.find('\n') != std::string::npos ||
           s.find('\r') != std::string::npos;
}

std::vector<std::string> splitBar(const std::string& line) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : line) {
        if (c == '|') { parts.push_back(cur); cur.clear(); }
        else cur += c;
    }
    parts.push_back(cur);
    return parts;
}

bool parseLongLong(const std::string& s, long long& out) {
    if (s.empty()) return false;
    try {
        size_t pos = 0;
        out = std::stoll(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

bool parseInt(const std::string& s, int& out) {
    long long v;
    if (!parseLongLong(s, v) || v < -2147483647LL || v > 2147483647LL) return false;
    out = static_cast<int>(v);
    return true;
}

// seatId: chu hoa A-Z + so >= 1 (vd A1, B12).
bool validSeatId(const std::string& s) {
    if (s.size() < 2 || s[0] < 'A' || s[0] > 'Z') return false;
    if (s[1] == '0') return false;
    for (size_t i = 1; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    return s.size() <= 10;
}

const char* statusToText(TicketStatus s) {
    switch (s) {
        case TicketStatus::Pending:   return "Pending";
        case TicketStatus::Paid:      return "Paid";
        case TicketStatus::Cancelled: return "Cancelled";
    }
    return "";
}

bool textToStatus(const std::string& t, TicketStatus& out) {
    if (t == "Pending")   { out = TicketStatus::Pending;   return true; }
    if (t == "Paid")      { out = TicketStatus::Paid;      return true; }
    if (t == "Cancelled") { out = TicketStatus::Cancelled; return true; }
    return false;
}

// Doc tat ca dong du lieu (bo dong rong, dong '#'). Tra false + err neu khong mo duoc.
bool readDataLines(const std::string& path, std::vector<std::pair<int, std::string>>& lines,
                   std::string& err) {
    std::ifstream in(path, std::ios::binary);
    if (!in) { err = "Khong mo duoc file: " + path; return false; }
    std::string line;
    int no = 0;
    while (std::getline(in, line)) {
        ++no;
        if (no == 1 && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);  // bo BOM UTF-8
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        lines.emplace_back(no, line);
    }
    if (in.bad()) { err = "Loi doc file: " + path; return false; }
    return true;
}

// Ghi vao file tam roi doi ten de khong lam hong file cu neu ghi that bai.
bool atomicWrite(const std::string& path, const std::string& content, std::string& err) {
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) { err = "Khong tao duoc file tam: " + tmp; return false; }
        out << content;
        out.flush();
        if (!out) {
            out.close();
            std::error_code ignored;
            std::filesystem::remove(tmp, ignored);
            err = "Ghi file tam that bai: " + tmp;
            return false;
        }
    }
    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        err = "Khong thay duoc file dich: " + path;
        return false;
    }
    return true;
}

std::string lineErr(int no, const std::string& msg) {
    return "Dong " + std::to_string(no) + ": " + msg;
}

}  // namespace

const std::string& FileStorage::lastError() const { return lastError_; }

bool FileStorage::saveMovies(const std::vector<Movie>& movies, const std::string& path) {
    lastError_.clear();
    std::ostringstream os;
    os << "# id|title|durationMinutes|genre|basePriceVnd\n";
    for (const Movie& m : movies) {
        if (hasForbiddenChar(m.getTitle()) || hasForbiddenChar(m.getGenre())) {
            lastError_ = "Phim id " + std::to_string(m.getId()) + " chua ky tu '|' hoac xuong dong";
            return false;
        }
        os << m.getId() << '|' << m.getTitle() << '|' << m.getDurationMinutes() << '|'
           << m.getGenre() << '|' << m.getBasePriceVnd() << '\n';
    }
    return atomicWrite(path, os.str(), lastError_);
}

bool FileStorage::loadMovies(std::vector<Movie>& movies, const std::string& path) {
    lastError_.clear();
    std::vector<std::pair<int, std::string>> lines;
    if (!readDataLines(path, lines, lastError_)) return false;

    std::vector<Movie> result;
    std::set<int> ids;
    for (const auto& [no, line] : lines) {
        auto c = splitBar(line);
        int id = 0, dur = 0;
        long long price = 0;
        if (c.size() != 5) { lastError_ = lineErr(no, "can 5 cot"); return false; }
        if (!parseInt(c[0], id) || id <= 0) { lastError_ = lineErr(no, "id khong hop le"); return false; }
        if (!ids.insert(id).second) { lastError_ = lineErr(no, "id phim trung"); return false; }
        if (c[1].empty() || c[3].empty()) { lastError_ = lineErr(no, "title/genre rong"); return false; }
        if (!parseInt(c[2], dur) || dur <= 0) { lastError_ = lineErr(no, "thoi luong phai > 0"); return false; }
        if (!parseLongLong(c[4], price) || price < 0) { lastError_ = lineErr(no, "gia phai >= 0"); return false; }
        result.emplace_back(id, c[1], dur, c[3], price);
    }
    movies = std::move(result);  // chi thay khi toan bo file hop le
    return true;
}

bool FileStorage::saveTickets(const std::vector<Ticket>& tickets, const std::string& path) {
    lastError_.clear();
    std::ostringstream os;
    os << "# id|customerId|showtimeId|seatId|priceVnd|TicketStatus\n";
    for (const Ticket& t : tickets) {
        if (hasForbiddenChar(t.getSeatId())) {
            lastError_ = "Ve id " + std::to_string(t.getId()) + " co seatId chua ky tu cam";
            return false;
        }
        os << t.getId() << '|' << t.getCustomerId() << '|' << t.getShowtimeId() << '|'
           << t.getSeatId() << '|' << t.getPriceVnd() << '|' << statusToText(t.getStatus()) << '\n';
    }
    return atomicWrite(path, os.str(), lastError_);
}

bool FileStorage::loadTickets(std::vector<Ticket>& tickets, const std::string& path) {
    lastError_.clear();
    std::vector<std::pair<int, std::string>> lines;
    if (!readDataLines(path, lines, lastError_)) return false;

    std::vector<Ticket> result;
    std::set<int> ids;
    for (const auto& [no, line] : lines) {
        auto c = splitBar(line);
        int id = 0, cust = 0, show = 0;
        long long price = 0;
        TicketStatus st = TicketStatus::Pending;
        if (c.size() != 6) { lastError_ = lineErr(no, "can 6 cot"); return false; }
        if (!parseInt(c[0], id) || id <= 0) { lastError_ = lineErr(no, "id ve khong hop le"); return false; }
        if (!ids.insert(id).second) { lastError_ = lineErr(no, "id ve trung"); return false; }
        if (!parseInt(c[1], cust) || cust <= 0) { lastError_ = lineErr(no, "customerId khong hop le"); return false; }
        if (!parseInt(c[2], show) || show <= 0) { lastError_ = lineErr(no, "showtimeId khong hop le"); return false; }
        if (!validSeatId(c[3])) { lastError_ = lineErr(no, "seatId khong hop le"); return false; }
        if (!parseLongLong(c[4], price) || price < 0) { lastError_ = lineErr(no, "gia phai >= 0"); return false; }
        if (!textToStatus(c[5], st)) { lastError_ = lineErr(no, "TicketStatus khong hop le"); return false; }
        result.emplace_back(id, cust, show, c[3], price, st);
    }
    // Invariant API_CONTRACT: toi da 1 ve chua Cancelled cho moi (showtimeId, seatId).
    std::set<std::pair<int, std::string>> active;
    for (const Ticket& t : result) {
        if (t.getStatus() == TicketStatus::Cancelled) continue;
        if (!active.insert({t.getShowtimeId(), t.getSeatId()}).second) {
            lastError_ = "Ve id " + std::to_string(t.getId()) +
                         " trung ghe dang hoat dong cung suat chieu";
            return false;
        }
    }
    tickets = std::move(result);
    return true;
}
