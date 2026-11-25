#include "main_window.h"
#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <iostream>
#include <stdexcept>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error( \
    std::string("Line ") + std::to_string(__LINE__) + ": " + #condition); } while (false)

static QPushButton* visibleSeat(MainWindow& window, const QString& id) {
    for (QPushButton* button : window.findChildren<QPushButton*>("seat"))
        if (button->isVisible() && button->text() == id) return button;
    return nullptr;
}

static void bookPending(QApplication& app, MainWindow& window) {
    window.showPage(2);
    app.processEvents();
    QPushButton* seat = visibleSeat(window, "A1");
    CHECK(seat && seat->isEnabled());
    seat->click();
    auto* method = window.findChild<QComboBox*>("bookingMethodSelect");
    auto* book = window.findChild<QPushButton*>("bookButton");
    CHECK(method && book && book->isEnabled());
    method->setCurrentIndex(1);
    book->click();
    app.processEvents();
    CHECK(window.currentPage() == 4);
    window.showPage(5);
    CHECK(window.findChild<QComboBox*>("pendingTicketSelect")->count() == 1);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        CHECK(argc == 3);
        const QString mode = QString::fromLocal8Bit(argv[1]);
        MainWindow window(QString::fromLocal8Bit(argv[2]));
        window.show();
        app.processEvents();
        auto* pending = window.findChild<QComboBox*>("pendingTicketSelect");
        CHECK(pending);
        if (mode == "write") {
            CHECK(pending->count() == 0);
            bookPending(app, window);
            window.showPage(3);
            auto* id = window.findChild<QLineEdit*>("customerIdInput");
            auto* name = window.findChild<QLineEdit*>("customerNameInput");
            CHECK(id && name);
            id->setText("2"); name->setText("Khách Lưu");
            QPushButton* add = nullptr;
            for (QPushButton* button : window.findChild<QWidget*>("customerPage")
                                            ->findChildren<QPushButton*>())
                if (button->text() == "Thêm khách") add = button;
            CHECK(add);
            add->click();
            CHECK(window.findChild<QComboBox*>("customerSelect")->count() == 2);
            CHECK(window.saveData());
        } else if (mode == "read") {
            CHECK(pending->count() == 1);
            CHECK(window.findChild<QComboBox*>("customerSelect")->count() == 2);
            window.showPage(2);
            app.processEvents();
            CHECK(visibleSeat(window, "A1") &&
                  visibleSeat(window, "A1")->property("seatState") == "held");
            window.showPage(5);
            QPushButton* pay = nullptr;
            for (QPushButton* button : window.findChild<QWidget*>("paymentPage")
                                            ->findChildren<QPushButton*>())
                if (button->text() == "Xử lý") pay = button;
            CHECK(pay);
            pay->click();
            CHECK(pending->count() == 0);
            CHECK(window.saveData());
        } else if (mode == "verify") {
            CHECK(pending->count() == 0);
            CHECK(window.findChild<QComboBox*>("customerSelect")->count() == 2);
            window.showPage(2);
            app.processEvents();
            CHECK(visibleSeat(window, "A1") &&
                  visibleSeat(window, "A1")->property("seatState") == "booked");
        } else if (mode == "room") {
            window.showPage(0);
            QPushButton* movie = nullptr;
            for (QPushButton* button : window.findChildren<QPushButton*>("movieButton"))
                if (button->property("movieId").toInt() == 2) movie = button;
            CHECK(movie);
            movie->click();
            CHECK(window.currentPage() == 1);
            QPushButton* show = nullptr;
            for (QPushButton* button : window.findChild<QWidget*>("showtimePage")
                                            ->findChildren<QPushButton*>("showtimeButton"))
                if (button->isVisible() && button->property("showtimeId").toInt() == 2)
                    show = button;
            CHECK(show);
            show->click();
            app.processEvents();
            CHECK(window.findChild<QComboBox*>("showtimeSelect")->currentData().toInt() == 2);
            CHECK(window.findChild<QLabel*>("roomName")->text().contains("Phòng Hai"));
            CHECK(visibleSeat(window, "C1") && !visibleSeat(window, "A1"));
        } else if (mode == "bad") {
            auto* toast = window.findChild<QLabel*>("toast");
            CHECK(toast && toast->text().contains("Bản lưu lỗi"));
            CHECK(pending->count() == 0);
            bookPending(app, window);
            CHECK(!window.loadData());
            CHECK(pending->count() == 1);
            window.showPage(2);
            app.processEvents();
            CHECK(visibleSeat(window, "A1") &&
                  visibleSeat(window, "A1")->property("seatState") == "held");
        } else throw std::runtime_error("Unknown mode");
        std::cout << "test_qt_persistence " << mode.toStdString() << " OK\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "test_qt_persistence failed: " << error.what() << '\n';
        return 1;
    }
}
