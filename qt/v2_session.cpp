#include "v2_session.h"
#include "bank_transfer_payment.h"
#include "demo_payment.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>

#include <algorithm>
#include <climits>
#include <set>

namespace {
std::string nativePath(const QString& path) {
    return path.toStdString(); // Qt trên Windows dùng UTF-8; filesystem::u8path ở core.
}

QString fileAt(const QString& directory, const QString& name) {
    return QDir(directory).filePath(name);
}

QString moneyError(const std::string& error) {
    return QString::fromUtf8(error.c_str());
}

const CinemaRoom* roomFor(const CinemaV2& cinema, int id) {
    for (const auto& room : cinema.rooms()) if (room.getId() == id) return &room;
    return nullptr;
}
}

V2Session::V2Session(const QString& movieFile, const QString& cinemaFile,
                     const QString& storageDirectory)
    : movieFile_(movieFile), cinemaFile_(cinemaFile), directory_(storageDirectory) {
    snacks_ = {{"Bắp rang bơ", 45000}, {"Bắp caramel", 55000},
               {"Nước ngọt", 30000}, {"Trà đào", 35000},
               {"Combo một người", 65000}, {"Combo hai người", 115000}};
    vouchers_ = {{"OOP10", "Giảm 10%, tối đa 40.000 ₫", 10, 40000},
                 {"SINHVIEN", "Giảm 15%, tối đa 50.000 ₫", 15, 50000},
                 {"CUOITUAN", "Giảm 8%, tối đa 30.000 ₫", 8, 30000}};

    const QStringList required = {"movie_details.txt", "cinema_v2.txt",
                                  "customers.txt", "tickets.txt", "orders.json"};
    int existing = 0;
    for (const auto& name : required) if (QFileInfo::exists(fileAt(directory_, name))) ++existing;
    if (existing == 0) {
        ready_ = loadDemo();
        notice_ = "Đang dùng dữ liệu demo. Chọn Lưu để giữ đơn hàng sau khi thoát.";
    } else if (existing == required.size()) {
        ready_ = load();
        if (!ready_) {
            const QString savedError = error_;
            ready_ = loadDemo();
            protectedSave_ = true;
            notice_ = "Bản lưu lỗi; dữ liệu demo đang chạy, bản lưu được giữ nguyên. " + savedError;
        }
    } else {
        ready_ = loadDemo();
        protectedSave_ = true;
        notice_ = "Bản lưu thiếu file; dữ liệu demo đang chạy và chưa ghi đè bản lưu.";
    }
}

void V2Session::resetBooking() {
    booking_ = std::make_unique<BookingManager>(cinema_.schedule(), cinema_.rooms(),
                                                users_, tickets_);
}

bool V2Session::loadDemo() {
    std::string error;
    MovieDetailsCatalog movies;
    CinemaV2 cinema;
    if (!movies.load(nativePath(movieFile_), error) ||
        !cinema.load(nativePath(cinemaFile_), movies, error)) {
        error_ = moneyError(error);
        return false;
    }
    booking_.reset();
    movies_ = std::move(movies);
    featuredIds_.clear();
    for (const auto& movie : movies_.all())
        if (!movie.posterAsset.empty()) featuredIds_.insert(movie.movieId);
    cinema_ = std::move(cinema);
    users_ = UserManager{};
    tickets_.clear();
    orders_.clear();
    resetBooking();
    error_.clear();
    dirty_ = true;
    return true;
}

std::vector<const MovieDetails*> V2Session::featuredMovies() const {
    std::vector<const MovieDetails*> result;
    for (const auto& movie : movies_.all())
        if (isFeaturedMovie(movie.movieId)) result.push_back(&movie);
    return result;
}

bool V2Session::isFeaturedMovie(int movieId) const {
    return featuredIds_.count(movieId) != 0 && movies_.find(movieId) != nullptr;
}

SeatStatus V2Session::seatStatus(int showtimeId, const std::string& seatId) const {
    return booking_ ? booking_->getSeatStatus(showtimeId, seatId) : SeatStatus::Booked;
}

long long V2Session::ticketPrice(int showtimeId, const std::string& seatId) const {
    const Showtime* show = cinema_.schedule().findShowtime(showtimeId);
    const RoomSeatLayout* layout = show ? cinema_.layout(show->getRoomId()) : nullptr;
    const SeatPosition* seat = layout ? layout->find(seatId) : nullptr;
    return show && seat ? show->getPriceVnd() +
        (seat->category == SeatCategory::VIP ? 20000 : 0) : 0;
}

long long V2Session::snackTotal(const std::vector<int>& quantities) const {
    if (quantities.size() != snacks_.size()) return -1;
    long long total = 0;
    for (size_t i = 0; i < quantities.size(); ++i) {
        if (quantities[i] < 0 || quantities[i] > 9) return -1;
        total += quantities[i] * snacks_[i].price;
    }
    return total;
}

long long V2Session::discountFor(const QString& code, long long subtotal) const {
    if (code.trimmed().isEmpty()) return 0;
    for (const auto& voucher : vouchers_) {
        if (voucher.code == code.trimmed().toUpper())
            return std::min(subtotal, std::min(voucher.cap,
                           subtotal * voucher.percent / 100));
    }
    return -1;
}

bool V2Session::checkout(int showtimeId, const std::vector<std::string>& seatIds,
                         const std::vector<int>& quantities, const QString& voucher,
                         const QString& buyer, const QString& phone, const QString& email,
                         bool payNow, QString& bookingCode) {
    error_.clear();
    const Showtime* show = cinema_.schedule().findShowtime(showtimeId);
    const CinemaBranch* branch = show ? cinema_.branches().findByRoom(show->getRoomId()) : nullptr;
    const long long food = snackTotal(quantities);
    if (!ready_ || !show || !branch || !isFeaturedMovie(show->getMovieId()) ||
        !cinema_.isBookable(showtimeId) ||
        seatIds.empty() || seatIds.size() > 8 || food < 0 ||
        buyer.trimmed().size() < 2 || phone.trimmed().size() < 9 ||
        !email.contains('@')) {
        error_ = "Kiểm tra suất chiếu, ghế và thông tin người mua.";
        return false;
    }
    std::set<std::string> uniqueSeats;
    long long seatsTotal = 0;
    for (const auto& seat : seatIds) {
        const long long price = ticketPrice(showtimeId, seat);
        if (!uniqueSeats.insert(seat).second || price <= 0 ||
            seatStatus(showtimeId, seat) != SeatStatus::Available) {
            error_ = "Ghế đã được giữ, đã đặt hoặc không thuộc phòng chiếu.";
            return false;
        }
        seatsTotal += price;
    }
    const long long discount = discountFor(voucher, seatsTotal + food);
    if (discount < 0) { error_ = "Mã ưu đãi không hợp lệ."; return false; }
    int nextCustomer = 1;
    for (const auto& customer : users_.getCustomers())
        nextCustomer = std::max(nextCustomer, customer.getId() + 1);
    if (nextCustomer == INT_MAX || !users_.addCustomer(Customer(nextCustomer,
                                          buyer.trimmed().toUtf8().toStdString()))) {
        error_ = "Không tạo được thông tin người mua.";
        return false;
    }
    // Đặt toàn bộ ghế ở Pending trước; nếu một ghế thất bại, hủy các vé đã tạo.
    BankTransferPayment pendingPayment;
    std::vector<int> created;
    for (const auto& seat : seatIds) {
        if (!cinema_.bookTicket(*booking_, nextCustomer, showtimeId, seat,
                                pendingPayment)) {
            for (int id : created) booking_->cancelTicket(id);
            error_ = "Không thể giữ đủ các ghế. Vui lòng chọn lại.";
            dirty_ = true;
            return false;
        }
        created.push_back(tickets_.back().getId());
    }
    CinemaOrder order;
    order.code = QString("OOP-%1").arg(created.front(), 6, 10, QChar('0'));
    order.buyer = buyer.trimmed();
    order.phone = phone.trimmed();
    order.email = email.trimmed();
    order.movieId = show->getMovieId();
    order.branchId = branch->id;
    order.showtimeId = showtimeId;
    order.ticketIds = created;
    for (const auto& seat : seatIds) order.seatIds.push_back(QString::fromStdString(seat));
    order.snackQuantities = quantities;
    order.voucher = voucher.trimmed().toUpper();
    order.ticketTotal = seatsTotal;
    order.snackTotal = food;
    order.discount = discount;
    orders_.push_back(order);
    bookingCode = order.code;
    dirty_ = true;
    if (payNow && !pay(order.code)) {
        error_ = "Đơn đã tạo ở trạng thái Pending; thanh toán demo chưa hoàn tất.";
    }
    return true;
}

const CinemaOrder* V2Session::findOrder(const QString& code) const {
    for (const auto& order : orders_) if (order.code == code) return &order;
    return nullptr;
}
CinemaOrder* V2Session::findOrder(const QString& code) {
    for (auto& order : orders_) if (order.code == code) return &order;
    return nullptr;
}

bool V2Session::pay(const QString& code) {
    const auto* order = findOrder(code);
    if (!order) { error_ = "Không tìm thấy đơn."; return false; }
    DemoPayment demo;
    bool changed = false;
    for (int id : order->ticketIds) {
        for (const Ticket& ticket : tickets_) if (ticket.getId() == id &&
            ticket.getStatus() == TicketStatus::Pending) {
            if (!booking_->payTicket(id, demo)) {
                error_ = "Không thể thanh toán vé Pending.";
                return false;
            }
            changed = true;
        }
    }
    if (changed) dirty_ = true;
    return changed;
}

bool V2Session::cancel(const QString& code) {
    const auto* order = findOrder(code);
    if (!order) { error_ = "Không tìm thấy đơn."; return false; }
    bool changed = false;
    for (int id : order->ticketIds) {
        for (const Ticket& ticket : tickets_) if (ticket.getId() == id &&
            ticket.getStatus() != TicketStatus::Cancelled) {
            changed = booking_->cancelTicket(id) || changed;
            break;
        }
    }
    if (changed) dirty_ = true;
    return changed;
}

bool V2Session::validateTickets(const CinemaV2& cinema, const UserManager& users,
                                const std::vector<Ticket>& tickets) const {
    std::set<std::pair<int, std::string>> occupied;
    for (const Ticket& ticket : tickets) {
        const Showtime* show = cinema.schedule().findShowtime(ticket.getShowtimeId());
        const CinemaRoom* room = show ? roomFor(cinema, show->getRoomId()) : nullptr;
        if (!show || !room || !room->hasSeat(ticket.getSeatId()) ||
            !users.findCustomer(ticket.getCustomerId()) ||
            ticket.getPriceVnd() != show->getPriceVnd()) return false;
        if (ticket.getStatus() != TicketStatus::Cancelled &&
            !occupied.insert({ticket.getShowtimeId(), ticket.getSeatId()}).second)
            return false;
    }
    return true;
}

bool V2Session::writeOrders(const QString& path) const {
    QJsonArray array;
    for (const auto& order : orders_) {
        QJsonObject row{{"code", order.code}, {"buyer", order.buyer},
                        {"phone", order.phone}, {"email", order.email},
                        {"movieId", order.movieId}, {"branchId", order.branchId},
                        {"showtimeId", order.showtimeId},
                        {"ticketTotal", double(order.ticketTotal)},
                        {"snackTotal", double(order.snackTotal)},
                        {"discount", double(order.discount)},
                        {"voucher", order.voucher}};
        QJsonArray ids, seats, food;
        for (int id : order.ticketIds) ids.append(id);
        for (const auto& seat : order.seatIds) seats.append(seat);
        for (int quantity : order.snackQuantities) food.append(quantity);
        row["ticketIds"] = ids; row["seatIds"] = seats; row["snacks"] = food;
        array.append(row);
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    const QByteArray json = QJsonDocument(array).toJson();
    if (file.write(json) != json.size()) return false;
    return file.commit();
}

bool V2Session::readOrders(const QString& path, std::vector<CinemaOrder>& orders) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonParseError parse;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parse);
    if (parse.error != QJsonParseError::NoError || !doc.isArray()) return false;
    QSet<QString> codes;
    for (const auto& value : doc.array()) {
        if (!value.isObject()) return false;
        const QJsonObject row = value.toObject();
        CinemaOrder order;
        order.code = row.value("code").toString();
        order.buyer = row.value("buyer").toString();
        order.phone = row.value("phone").toString();
        order.email = row.value("email").toString();
        order.movieId = row.value("movieId").toInt();
        order.branchId = row.value("branchId").toInt();
        order.showtimeId = row.value("showtimeId").toInt();
        order.ticketTotal = static_cast<long long>(row.value("ticketTotal").toDouble(-1));
        order.snackTotal = static_cast<long long>(row.value("snackTotal").toDouble(-1));
        order.discount = static_cast<long long>(row.value("discount").toDouble(-1));
        order.voucher = row.value("voucher").toString();
        if (order.code.isEmpty() || codes.contains(order.code) ||
            order.buyer.isEmpty() || order.phone.isEmpty() || order.email.isEmpty() ||
            order.movieId <= 0 || order.branchId <= 0 || order.showtimeId <= 0 ||
            order.ticketTotal < 0 || order.snackTotal < 0 || order.discount < 0 ||
            !row.value("ticketIds").isArray() || !row.value("seatIds").isArray() ||
            !row.value("snacks").isArray()) return false;
        codes.insert(order.code);
        for (const auto& id : row.value("ticketIds").toArray()) {
            if (!id.isDouble() || id.toInt() <= 0) return false;
            order.ticketIds.push_back(id.toInt());
        }
        for (const auto& seat : row.value("seatIds").toArray()) {
            if (!seat.isString()) return false;
            order.seatIds.push_back(seat.toString());
        }
        for (const auto& quantity : row.value("snacks").toArray()) {
            if (!quantity.isDouble() || quantity.toInt() < 0 || quantity.toInt() > 9)
                return false;
            order.snackQuantities.push_back(quantity.toInt());
        }
        if (order.ticketIds.empty() || order.ticketIds.size() != order.seatIds.size() ||
            order.snackQuantities.size() != snacks_.size()) return false;
        orders.push_back(std::move(order));
    }
    return true;
}

bool V2Session::load() {
    error_.clear();
    std::string coreError;
    MovieDetailsCatalog movies;
    CinemaV2 cinema;
    FileStorage storage;
    std::vector<Customer> customers;
    std::vector<Ticket> tickets;
    std::vector<CinemaOrder> orders;
    MovieDetailsCatalog currentCatalog;
    if (!currentCatalog.load(nativePath(movieFile_), coreError)) {
        error_ = moneyError(coreError); return false;
    }
    if (!movies.load(nativePath(fileAt(directory_, "movie_details.txt")), coreError) ||
        !cinema.load(nativePath(fileAt(directory_, "cinema_v2.txt")), movies, coreError)) {
        error_ = moneyError(coreError); return false;
    }
    if (!storage.loadCustomers(customers, nativePath(fileAt(directory_, "customers.txt"))) ||
        !storage.loadTickets(tickets, nativePath(fileAt(directory_, "tickets.txt")))) {
        error_ = moneyError(storage.lastError()); return false;
    }
    UserManager users;
    for (const auto& customer : customers) if (!users.addCustomer(customer)) {
        error_ = "Khách hàng bị trùng."; return false;
    }
    if (!validateTickets(cinema, users, tickets) ||
        !readOrders(fileAt(directory_, "orders.json"), orders)) {
        error_ = "Bản lưu có tham chiếu hoặc đơn hàng không hợp lệ."; return false;
    }
    std::set<int> orderedTickets;
    for (const auto& order : orders) {
        const Showtime* show = cinema.schedule().findShowtime(order.showtimeId);
        const CinemaBranch* branch = show ? cinema.branches().findByRoom(show->getRoomId()) : nullptr;
        if (!show || !branch || show->getMovieId() != order.movieId ||
            branch->id != order.branchId) {
            error_ = "Đơn hàng tham chiếu phim, rạp hoặc suất sai."; return false;
        }
        for (size_t i = 0; i < order.ticketIds.size(); ++i) {
            const auto it = std::find_if(tickets.begin(), tickets.end(), [&](const Ticket& ticket) {
                return ticket.getId() == order.ticketIds[i];
            });
            if (it == tickets.end() || it->getShowtimeId() != order.showtimeId ||
                QString::fromStdString(it->getSeatId()) != order.seatIds[i] ||
                !orderedTickets.insert(it->getId()).second) {
                error_ = "Đơn hàng tham chiếu vé sai hoặc trùng."; return false;
            }
        }
        long long seatsTotal = 0;
        for (const auto& seat : order.seatIds) {
            const RoomSeatLayout* layout = cinema.layout(show->getRoomId());
            const SeatPosition* position = layout ? layout->find(seat.toStdString()) : nullptr;
            if (!position) { error_ = "Ghế trong đơn không hợp lệ."; return false; }
            seatsTotal += show->getPriceVnd() +
                (position->category == SeatCategory::VIP ? 20000 : 0);
        }
        if (seatsTotal != order.ticketTotal ||
            snackTotal(order.snackQuantities) != order.snackTotal ||
            discountFor(order.voucher, seatsTotal + order.snackTotal) != order.discount) {
            error_ = "Tổng tiền hoặc ưu đãi trong đơn không hợp lệ."; return false;
        }
    }
    // Giữ metadata/ID lịch sử; thêm phim mới và poster hiện tại mà không đổi thời lượng
    // của phim đã có suất trong bản lưu cũ.
    MovieDetailsCatalog combined;
    for (const auto& saved : movies.all()) {
        MovieDetails record = saved;
        if (const auto* current = currentCatalog.find(saved.movieId))
            record.posterAsset = current->posterAsset;
        if (!combined.add(record, coreError)) { error_ = moneyError(coreError); return false; }
    }
    for (const auto& current : currentCatalog.all())
        if (!combined.find(current.movieId) && !combined.add(current, coreError)) {
            error_ = moneyError(coreError); return false;
        }
    booking_.reset();
    movies_ = std::move(combined);
    featuredIds_.clear();
    for (const auto& current : currentCatalog.all())
        if (!current.posterAsset.empty()) featuredIds_.insert(current.movieId);
    cinema_ = std::move(cinema);
    users_ = std::move(users);
    tickets_ = std::move(tickets);
    orders_ = std::move(orders);
    resetBooking();
    ready_ = true;
    dirty_ = false;
    protectedSave_ = false;
    return true;
}

bool V2Session::save(bool allowDamagedOverwrite) {
    error_.clear();
    if (protectedSave_ && !allowDamagedOverwrite) {
        error_ = "Bản lưu lỗi hoặc thiếu file được bảo vệ. Cần xác nhận và sao lưu trước khi thay thế.";
        return false;
    }
    if (!ready_ || !QDir().mkpath(directory_)) {
        error_ = "Không tạo được thư mục lưu."; return false;
    }
    if (protectedSave_) {
        const QString backup = fileAt(directory_, "damaged-backup-" +
            QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmsszzz"));
        if (!QDir().mkpath(backup)) {
            error_ = "Không thể tạo bản sao dữ liệu lỗi."; return false;
        }
        for (const QString& name : {"movie_details.txt", "cinema_v2.txt",
                                    "customers.txt", "tickets.txt", "orders.json"}) {
            const QString source = fileAt(directory_, name);
            if (QFileInfo::exists(source) && !QFile::copy(source, fileAt(backup, name))) {
                error_ = "Không thể sao lưu file lỗi: " + name; return false;
            }
        }
    }
    // Bản lưu V2 ở thư mục riêng; mỗi file dùng ghi tạm rồi thay thế.
    std::string coreError;
    FileStorage storage;
    if (!movies_.save(nativePath(fileAt(directory_, "movie_details.txt")), coreError) ||
        !cinema_.save(nativePath(fileAt(directory_, "cinema_v2.txt")), coreError)) {
        error_ = moneyError(coreError); return false;
    }
    if (!storage.saveCustomers(users_.getCustomers(),
            nativePath(fileAt(directory_, "customers.txt"))) ||
        !storage.saveTickets(tickets_, nativePath(fileAt(directory_, "tickets.txt")))) {
        error_ = moneyError(storage.lastError()); return false;
    }
    if (!writeOrders(fileAt(directory_, "orders.json"))) {
        error_ = "Không ghi được đơn hàng."; return false;
    }
    dirty_ = false;
    protectedSave_ = false;
    return true;
}
