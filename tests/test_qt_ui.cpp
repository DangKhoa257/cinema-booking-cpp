#include "main_window.h"
#include <QApplication>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <cassert>

static QPushButton* seatButton(MainWindow& window, const QString& id) {
    for (QPushButton* button : window.findChildren<QPushButton*>("seat"))
        if (button->isVisible() && button->text() == id) return button;
    return nullptr;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    assert(window.currentPage() == 0);
    window.show();
    app.processEvents();
    if (qEnvironmentVariableIsSet("CINEMA_QT_HOME_SCREENSHOT"))
        assert(window.grab().save(qEnvironmentVariable("CINEMA_QT_HOME_SCREENSHOT")));
    const char* pages[] = {"homePage", "showtimePage", "seatMapPage",
                           "customerPage", "ticketPage", "paymentPage"};
    for (int i = 0; i < 6; ++i) {
        window.showPage(i);
        assert(window.currentPage() == i);
        assert(window.findChild<QWidget*>(pages[i]) != nullptr);
    }
    window.showPage(2);
    auto* showtime = window.findChild<QComboBox*>("showtimeSelect");
    auto* customer = window.findChild<QComboBox*>("customerSelect");
    assert(showtime && showtime->count() == 2);
    assert(customer && customer->count() == 1);
    assert(window.findChildren<QPushButton*>("seat").size() == 4);
    app.processEvents();
    if (qEnvironmentVariableIsSet("CINEMA_QT_SCREENSHOT"))
        assert(window.grab().save(qEnvironmentVariable("CINEMA_QT_SCREENSHOT")));

    auto* seat = seatButton(window, "A1");
    auto* book = window.findChild<QPushButton*>("bookButton");
    assert(seat && book);
    seat->click();
    // Chon chuyen khoan demo: ve duoc tao Pending, khong duoc tu xac nhan.
    auto* bookingMethod = window.findChild<QComboBox*>("bookingMethodSelect");
    assert(bookingMethod);
    bookingMethod->setCurrentIndex(1);
    book->click();
    app.processEvents();
    assert(seatButton(window, "A1") && !seatButton(window, "A1")->isEnabled());
    window.showPage(5);
    auto* pending = window.findChild<QComboBox*>("pendingTicketSelect");
    assert(pending && pending->count() == 1);
    auto* paymentPage = window.findChild<QWidget*>("paymentPage");
    for (QPushButton* button : paymentPage->findChildren<QPushButton*>())
        if (button->text() == "Xử lý") button->click();
    app.processEvents();
    assert(pending->count() == 0);
    window.showPage(4);
    auto* ticketPage = window.findChild<QWidget*>("ticketPage");
    for (QPushButton* button : ticketPage->findChildren<QPushButton*>())
        if (button->text() == "Hủy vé") button->click();
    app.processEvents();
    window.showPage(2);
    assert(seatButton(window, "A1") && seatButton(window, "A1")->isEnabled());
    return 0;
}
