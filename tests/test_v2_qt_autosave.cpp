#include "v2_window.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
#include <stdexcept>

void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

QPushButton* button(V2Window& window, const char* name) {
    auto* result = window.findChild<QPushButton*>(name);
    require(result != nullptr, name);
    return result;
}

void acceptConfirmation(QPushButton* trigger) {
    bool accepted = false;
    QTimer timer;
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
        for (QWidget* top : QApplication::topLevelWidgets()) {
            auto* dialog = qobject_cast<QMessageBox*>(top);
            if (dialog && dialog->isVisible()) {
                accepted = true;
                timer.stop();
                if (auto* yes = dialog->button(QMessageBox::Yes)) yes->click();
                else dialog->done(QMessageBox::Yes);
                return;
            }
        }
    });
    timer.start();
    QTimer::singleShot(3000, &timer, [&] {
        if (accepted) return;
        for (QWidget* top : QApplication::topLevelWidgets())
            if (auto* dialog = qobject_cast<QMessageBox*>(top))
                if (dialog->isVisible()) dialog->done(QMessageBox::No);
    });
    trigger->click();
    timer.stop();
    require(accepted, "confirmation dialog appeared and was accepted");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        QTemporaryDir temp;
        require(temp.isValid(), "temporary data directory");
        const QString movie = QString::fromUtf8(MOVIE_DATA_FILE);
        const QString cinema = QString::fromUtf8(CINEMA_DATA_FILE);
        V2Window window(movie, cinema, QString::fromUtf8(ASSET_DIRECTORY), temp.path());
        window.show();
        app.processEvents();
        button(window, "movie-101")->click();
        button(window, "showtime-1")->click();
        button(window, "seat-A1")->click();
        button(window, "openSnacks")->click();
        button(window, "snack-plus-3")->click();
        button(window, "openCheckout")->click();
        for (auto* input : window.findChildren<QLineEdit*>()) {
            if (input->placeholderText() == "Họ và tên") input->setText("Khách kiểm thử");
            if (input->placeholderText() == "Số điện thoại") input->setText("0901234567");
            if (input->placeholderText() == "Email nhận mã vé") input->setText("test@example.com");
        }
        for (auto* option : window.findChildren<QRadioButton*>())
            if (option->text().contains("Pending")) option->setChecked(true);
        acceptConfirmation(button(window, "confirmCheckout"));
        require(window.session().orders().size() == 1 && !window.session().dirty(),
                "checkout automatically saved");
        require(window.session().orders().front().snackTotal == 99000,
                "combo quantity reaches saved order");
        const QString code = window.session().orders().front().code;
        require(QFileInfo::exists(QDir(temp.path()).filePath("orders.json")),
                "order file written without save button");
        V2Session afterBooking(movie, cinema, temp.path());
        require(afterBooking.ready() && afterBooking.orders().size() == 1 &&
                afterBooking.orders().front().code == code &&
                afterBooking.seatStatus(1, "A1") == SeatStatus::Held,
                "pending order restored after restart");
        button(window, "payOrder")->click();
        require(!window.session().dirty(), "demo payment automatically saved");
        V2Session afterPayment(movie, cinema, temp.path());
        require(afterPayment.ready() && afterPayment.seatStatus(1, "A1") == SeatStatus::Booked,
                "paid ticket restored after restart");
        acceptConfirmation(button(window, "cancelOrder"));
        require(!window.session().dirty(), "cancellation automatically saved");
        V2Session afterCancel(movie, cinema, temp.path());
        require(afterCancel.ready() && afterCancel.seatStatus(1, "A1") == SeatStatus::Available,
                "cancelled ticket and released seat restored");
        std::cout << "V2 Qt automatic persistence passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}
