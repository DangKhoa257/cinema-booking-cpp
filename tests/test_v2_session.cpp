#include "v2_session.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
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
        require(session.movies().all().size() == 7, "seven real movies");
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
        require(session.pay(code), "pending to paid");
        require(session.seatStatus(showId, "A1") == SeatStatus::Booked, "booked seat");
        require(session.save(), "save V2");
        V2Session reopened(QString::fromUtf8(MOVIE_DATA_FILE),
                           QString::fromUtf8(CINEMA_DATA_FILE), folder);
        require(reopened.ready() && reopened.orders().size() == 1, "load after restart");
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
        std::cout << "V2 session flow passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}
