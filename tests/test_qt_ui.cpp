#include "main_window.h"
#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTimer>
#include <stdexcept>
#include <iostream>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error( \
    std::string("Line ") + std::to_string(__LINE__) + ": " + #condition); } while (false)

static void answerDialog(QMessageBox::StandardButton answer) {
    QTimer::singleShot(20, [answer] {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            auto* box = qobject_cast<QMessageBox*>(widget);
            if (box && box->isVisible() && box->button(answer)) {
                box->button(answer)->click(); return;
            }
        }
    });
}

static QPushButton* seatButton(MainWindow& window, const QString& id) {
    for (QPushButton* button : window.findChildren<QPushButton*>("seat"))
        if (button->isVisible() && button->text() == id) return button;
    return nullptr;
}

static QPushButton* buttonWithId(QWidget* parent, const char* name,
                                  const char* property, int id) {
    for (QPushButton* button : parent->findChildren<QPushButton*>(name))
        if (button->isVisible() && button->property(property).toInt() == id) return button;
    return nullptr;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        QTemporaryDir temporary;
        CHECK(temporary.isValid());
        MainWindow window(temporary.path() + "/cinema");
        window.show();
        app.processEvents();
        CHECK(window.currentPage() == 0);
        if (qEnvironmentVariableIsSet("CINEMA_QT_HOME_SCREENSHOT"))
            CHECK(window.grab().save(qEnvironmentVariable("CINEMA_QT_HOME_SCREENSHOT")));

        auto* movie = buttonWithId(&window, "movieButton", "movieId", 2);
        CHECK(movie != nullptr);
        movie->click();
        CHECK(window.currentPage() == 1);
        auto* showPage = window.findChild<QWidget*>("showtimePage");
        CHECK(showPage != nullptr);
        int visibleShows = 0;
        for (QPushButton* button : showPage->findChildren<QPushButton*>("showtimeButton"))
            if (button->isVisible()) ++visibleShows;
        CHECK(visibleShows == 1);
        auto* show = buttonWithId(showPage, "showtimeButton", "showtimeId", 2);
        CHECK(show != nullptr);
        show->click();
        CHECK(window.currentPage() == 2);
        auto* showtime = window.findChild<QComboBox*>("showtimeSelect");
        auto* customer = window.findChild<QComboBox*>("customerSelect");
        auto* room = window.findChild<QLabel*>("roomName");
        auto* confirmation = window.findChild<QLabel*>("bookingConfirmation");
        auto* book = window.findChild<QPushButton*>("bookButton");
        CHECK(showtime && showtime->currentData().toInt() == 2);
        CHECK(customer && customer->currentData().toInt() == 1);
        CHECK(room && room->text().contains("Phòng Demo"));
        CHECK(book && !book->isEnabled());
        auto* seat = seatButton(window, "A1");
        CHECK(seat && seat->isEnabled());
        seat->click();
        CHECK(book->isEnabled());
        CHECK(confirmation && confirmation->text().contains("Phim Thứ Hai") &&
              confirmation->text().contains("2026-12-01 21:00") &&
              confirmation->text().contains("A1") &&
              confirmation->text().contains("Khách Demo") &&
              confirmation->text().contains("VND"));
        if (qEnvironmentVariableIsSet("CINEMA_QT_SCREENSHOT")) {
            app.processEvents();
            CHECK(window.grab().save(qEnvironmentVariable("CINEMA_QT_SCREENSHOT")));
        }
        window.resize(860, 620);
        app.processEvents();
        CHECK(window.width() == 860 && window.height() == 620);
        CHECK(window.findChildren<QScrollArea*>()[2]->verticalScrollBar()->maximum() > 0);
        if (qEnvironmentVariableIsSet("CINEMA_QT_SMALL_SCREENSHOT"))
            CHECK(window.grab().save(qEnvironmentVariable("CINEMA_QT_SMALL_SCREENSHOT")));
        window.resize(1120, 760);
        auto* bookingMethod = window.findChild<QComboBox*>("bookingMethodSelect");
        CHECK(bookingMethod);
        bookingMethod->setCurrentIndex(1);  // chuyen khoan demo
        answerDialog(QMessageBox::Yes);
        book->click();
        app.processEvents();
        CHECK(window.currentPage() == 4);
        CHECK(window.findChild<QLabel*>("saveStatus")->text().contains("Chưa lưu"));
        window.showPage(2);
        CHECK(seatButton(window, "A1") && !seatButton(window, "A1")->isEnabled());
        CHECK(seatButton(window, "A1")->property("seatState") == "held");
        CHECK(!book->isEnabled());
        showtime->setCurrentIndex(showtime->findData(1));
        app.processEvents();
        CHECK(seatButton(window, "A1") && seatButton(window, "A1")->isEnabled());
        showtime->setCurrentIndex(showtime->findData(2));
        app.processEvents();
        CHECK(!seatButton(window, "A1")->isEnabled());

        window.showPage(5);
        auto* pending = window.findChild<QComboBox*>("pendingTicketSelect");
        auto* paymentPage = window.findChild<QWidget*>("paymentPage");
        CHECK(pending && pending->count() == 1 && paymentPage);
        QPushButton* pay = nullptr;
        for (QPushButton* button : paymentPage->findChildren<QPushButton*>())
            if (button->text() == "Xử lý") pay = button;
        CHECK(pay);
        answerDialog(QMessageBox::Yes);
        pay->click();  // DemoPayment: Pending -> Paid
        CHECK(pending->count() == 0);
        window.showPage(2);
        CHECK(seatButton(window, "A1")->property("seatState") == "booked");
        window.showPage(4);
        auto* ticketPage = window.findChild<QWidget*>("ticketPage");
        QPushButton* cancel = nullptr;
        for (QPushButton* button : ticketPage->findChildren<QPushButton*>())
            if (button->text() == "Hủy vé") cancel = button;
        CHECK(cancel);
        answerDialog(QMessageBox::Yes);
        cancel->click();
        app.processEvents();
        window.showPage(2);
        CHECK(seatButton(window, "A1") && seatButton(window, "A1")->isEnabled());
        CHECK(seatButton(window, "A1")->property("seatState") == "available");
        CHECK(window.saveData());
        CHECK(window.findChild<QLabel*>("saveStatus")->text().contains("Đã lưu"));
        const QString unicodePath = temporary.path() + "/Dữ liệu";
        MainWindow unicodeSession(unicodePath);
        CHECK(unicodeSession.saveData());
        CHECK(unicodeSession.saveData());
        MainWindow restored(unicodePath);
        CHECK(restored.findChild<QLabel*>("saveStatus")->text().contains("Đã lưu"));
        MainWindow unsaved(temporary.path() + "/new-session");
        unsaved.show();
        app.processEvents();
        auto* loadButton = unsaved.findChild<QPushButton*>("loadDataButton");
        CHECK(loadButton);
        answerDialog(QMessageBox::Cancel);
        loadButton->click();
        CHECK(unsaved.findChild<QLabel*>("saveStatus")->text().contains("Chưa lưu"));
        answerDialog(QMessageBox::Cancel);
        unsaved.close();
        CHECK(unsaved.isVisible());
        answerDialog(QMessageBox::Save);
        unsaved.close();
        CHECK(!unsaved.isVisible());
        CHECK(QFile::exists(temporary.path() + "/new-session/movies.txt"));
        QFile blockedPath(temporary.path() + "/blocked-file");
        CHECK(blockedPath.open(QIODevice::WriteOnly));
        blockedPath.close();
        MainWindow blocked(blockedPath.fileName());
        CHECK(!blocked.saveData());
        CHECK(blocked.findChild<QLabel*>("saveStatus")->text().contains("thất bại"));
        std::cout << "test_qt_ui OK\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "test_qt_ui failed: " << error.what() << '\n';
        return 1;
    }
}
