#include "v2_session.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    try {
        QTemporaryDir temp;
        require(temp.isValid(), "temporary directory");
        const QString folder = QDir(temp.path()).filePath(QString::fromUtf8("Dữ liệu thử nghiệm"));
        V2Session session(QString::fromUtf8(MOVIE_DATA_FILE),
                          QString::fromUtf8(CINEMA_DATA_FILE), folder);
        require(session.ready(), "demo loads");
        const std::vector<QString> snackNames{"Bắp rang bơ", "Nước ngọt",
                                              "Combo 1", "Combo 2", "Combo 3"};
        const std::vector<QString> snackContents{"1 bắp", "1 nước",
                                                 "1 bắp + 2 nước", "2 bắp + 2 nước",
                                                 "3 bắp + 3 nước"};
        const std::vector<long long> snackPrices{45000, 30000, 99000, 139000, 199000};
        require(session.snacks().size() == 5, "exactly five current snack products");
        for (size_t i = 0; i < snackNames.size(); ++i)
            require(session.snacks()[i].id == int(i + 1) &&
                    session.snacks()[i].name == snackNames[i] &&
                    session.snacks()[i].contents == snackContents[i] &&
                    session.snacks()[i].price == snackPrices[i] &&
                    !session.snacks()[i].imageFile.isEmpty(), "snack catalog metadata");
        require(session.snackTotal({1, 1, 1, 1, 1}) == 512000,
                "five product prices add correctly");
        require(session.snackTotal({-1, 0, 0, 0, 0}) < 0 &&
                session.snackTotal({10, 0, 0, 0, 0}) < 0, "quantity bounds");
        QFile blockedDirectory(QDir(temp.path()).filePath("not-a-directory"));
        require(blockedDirectory.open(QIODevice::WriteOnly), "create blocked storage path");
        blockedDirectory.write("keep this file");
        blockedDirectory.close();
        V2Session cannotSave(QString::fromUtf8(MOVIE_DATA_FILE),
                             QString::fromUtf8(CINEMA_DATA_FILE),
                             blockedDirectory.fileName());
        QString unsavedCode;
        require(cannotSave.ready() &&
                cannotSave.checkout(1, {"A1"}, {0, 0, 0, 0, 0}, "",
                                    "Khách thử", "0901234567", "unsaved@example.com",
                                    false, unsavedCode) &&
                !cannotSave.save() && cannotSave.dirty() &&
                cannotSave.orders().size() == 1,
                "failed save leaves visible unsaved order and dirty state");
        require(session.movies().all().size() == 11, "nine displayed and two historical movies");
        require(session.featuredMovies().size() == 9, "nine poster movies are featured");
        require(!session.isFeaturedMovie(104) && !session.isFeaturedMovie(108),
                "movies without supplied posters remain historical only");
        for (const auto& item : session.cinema().schedule().getShowtimes())
            require(session.isFeaturedMovie(item.getMovieId()),
                    "new demo schedule uses only poster movies");
        const QString oldFolder = QDir(temp.path()).filePath("older-snapshot");
        V2Session oldVersion(QString::fromUtf8(LEGACY_MOVIE_DATA_FILE),
                             QString::fromUtf8(LEGACY_CINEMA_DATA_FILE), oldFolder);
        require(oldVersion.ready() && oldVersion.movies().all().size() == 7 &&
                oldVersion.save(), "old seven-movie snapshot saved");
        V2Session upgraded(QString::fromUtf8(MOVIE_DATA_FILE),
                           QString::fromUtf8(CINEMA_DATA_FILE), oldFolder);
        require(upgraded.ready() && upgraded.movies().all().size() == 11 &&
                upgraded.featuredMovies().size() == 9,
                "old snapshot gains new catalog without losing old IDs");
        const Showtime* historical = upgraded.cinema().schedule().findShowtime(4);
        require(historical && historical->getMovieId() == 104 &&
                !upgraded.isFeaturedMovie(104), "old movie schedule retained but hidden");
        QString blockedCode;
        require(!upgraded.checkout(4, {"A1"}, std::vector<int>(upgraded.snacks().size(), 0),
                                  "", "Khách cũ", "0901234567", "old@example.com",
                                  false, blockedCode), "no new booking for historical movie");
        require(session.cinema().branches().all().size() == 4, "four demo branches");
        for (const auto& room : session.cinema().rooms())
            require(room.getSeats().size() >= 80 && room.getSeats().size() <= 120,
                    "seat count 80-120");
        const auto& show = session.cinema().schedule().getShowtimes().front();
        const int showId = show.getId();
        require(session.cinema().isBookable(showId), "future showtime");
        std::vector<int> snacks(session.snacks().size(), 0);
        snacks[0] = 1;
        QString code;
        require(session.checkout(showId, {"A1", "G1"}, snacks, "OOP10",
                                 "Nguyễn Văn A", "0901234567", "a@example.com",
                                 false, code), "guest pending checkout");
        require(!code.isEmpty(), "booking code");
        require(session.seatStatus(showId, "A1") == SeatStatus::Held, "held seat");
        require(session.seatStatus(showId, "G1") == SeatStatus::Held, "held VIP seat");
        require(session.ticketPrice(showId, "G1") == show.getPriceVnd() + 20000,
                "VIP surcharge");
        require(session.orders().front().discount > 0, "voucher discount");
        QString duplicate;
        require(!session.checkout(showId, {"A1"}, snacks, "", "Nguyễn Văn B",
                                  "0901234568", "b@example.com", true, duplicate),
                "duplicate seat rejected");
        require(!session.pay("OOP-UNKNOWN") &&
                session.seatStatus(showId, "A1") == SeatStatus::Held,
                "failed payment lookup leaves pending tickets and held seat unchanged");
        require(session.pay(code), "pending to paid");
        require(session.seatStatus(showId, "A1") == SeatStatus::Booked, "booked seat");
        require(!session.pay(code) &&
                session.seatStatus(showId, "A1") == SeatStatus::Booked,
                "repeated payment does not alter paid tickets");
        require(session.save(), "save V2");
        // Bản V2 cũ lưu sáu số lượng; thêm món cũ số lượng 0 để mô phỏng file cũ.
        const QString ordersPath = QDir(folder).filePath("orders.json");
        QFile oldOrders(ordersPath);
        require(oldOrders.open(QIODevice::ReadOnly), "read saved order");
        QJsonArray rows = QJsonDocument::fromJson(oldOrders.readAll()).array();
        oldOrders.close();
        require(rows.size() == 1, "one saved order");
        QJsonObject oldRow = rows[0].toObject();
        QJsonArray oldSnacks = oldRow.value("snacks").toArray();
        oldSnacks.append(0);
        oldRow["snacks"] = oldSnacks;
        rows[0] = oldRow;
        require(oldOrders.open(QIODevice::WriteOnly | QIODevice::Truncate),
                "write legacy order format");
        oldOrders.write(QJsonDocument(rows).toJson());
        oldOrders.close();
        V2Session reopened(QString::fromUtf8(MOVIE_DATA_FILE),
                           QString::fromUtf8(CINEMA_DATA_FILE), folder);
        require(reopened.ready() && reopened.orders().size() == 1 &&
                reopened.orders().front().snackQuantities.size() == 6,
                "legacy six-item order loads after restart");
        require(reopened.seatStatus(showId, "A1") == SeatStatus::Booked,
                "paid seat restored");
        require(reopened.cancel(code), "paid to cancelled");
        require(reopened.seatStatus(showId, "A1") == SeatStatus::Available,
                "seat released");
        require(reopened.save(), "save cancellation");
        V2Session third(QString::fromUtf8(MOVIE_DATA_FILE),
                        QString::fromUtf8(CINEMA_DATA_FILE), folder);
        require(third.ready() && third.seatStatus(showId, "A1") == SeatStatus::Available,
                "cancel persists");
        QFile damaged(QDir(folder).filePath("tickets.txt"));
        require(damaged.open(QIODevice::WriteOnly | QIODevice::Truncate), "open damage file");
        damaged.write("not a valid ticket\n");
        damaged.close();
        require(!third.load(), "damaged save rejected");
        require(third.orders().size() == 1 &&
                third.seatStatus(showId, "A1") == SeatStatus::Available,
                "memory survives failed load");
        V2Session protectedSession(QString::fromUtf8(MOVIE_DATA_FILE),
                                   QString::fromUtf8(CINEMA_DATA_FILE), folder);
        require(protectedSession.ready() && protectedSession.hasDamagedSave(),
                "corrupt save shows demo without losing file");
        require(!protectedSession.save(), "damaged save cannot be overwritten silently");
        if (!protectedSession.save(true))
            throw std::runtime_error(("confirmed replacement: " + protectedSession.error()).toStdString());
        const auto backups = QDir(folder).entryList({"damaged-backup-*"}, QDir::Dirs | QDir::NoDotAndDotDot);
        require(backups.size() == 1, "damaged backup exists");
        QFile backup(QDir(folder).filePath(backups.front() + "/tickets.txt"));
        require(backup.open(QIODevice::ReadOnly) && backup.readAll().contains("not a valid ticket"),
                "damaged content preserved in backup");
        V2Session immediate(QString::fromUtf8(MOVIE_DATA_FILE),
                            QString::fromUtf8(CINEMA_DATA_FILE),
                            QDir(temp.path()).filePath("instant-payment"));
        QString instantCode;
        require(immediate.ready() &&
                immediate.checkout(showId, {"A2"}, snacks, "", "Nguyễn Văn C",
                                   "0901234569", "c@example.com", true, instantCode) &&
                immediate.error().isEmpty() &&
                immediate.seatStatus(showId, "A2") == SeatStatus::Booked,
                "successful immediate demo payment marks seat booked");
        std::cout << "V2 session flow passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}
