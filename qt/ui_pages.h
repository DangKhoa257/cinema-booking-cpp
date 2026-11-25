#pragma once

#include "ui_session.h"
#include <QWidget>
#include <functional>

class QComboBox;
class QGridLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class QVBoxLayout;

struct UiCallbacks {
    std::function<void(int)> navigate;
    std::function<void(int)> chooseMovie;
    std::function<void(int)> chooseShowtime;
    std::function<void(const QString&)> notify;
    std::function<void()> changed;
};

class HomePage : public QWidget {
public:
    HomePage(UiSession& session, UiCallbacks callbacks, QWidget* parent = nullptr);
    void refresh();
private:
    UiSession& session_;
    UiCallbacks callbacks_;
    QVBoxLayout* moviesLayout_;
};

class ShowtimePage : public QWidget {
public:
    ShowtimePage(UiSession& session, UiCallbacks callbacks, QWidget* parent = nullptr);
    void setMovieFilter(int movieId);
    int movieFilter() const { return movieFilter_; }
    void refresh();
private:
    UiSession& session_;
    UiCallbacks callbacks_;
    QVBoxLayout* showsLayout_;
    QLabel* filterLabel_;
    QPushButton* clearFilter_;
    int movieFilter_ = 0;
};

class SeatMapPage : public QWidget {
public:
    SeatMapPage(UiSession& session, UiCallbacks callbacks, QWidget* parent = nullptr);
    void selectShowtime(int showtimeId);
    void refresh();
private:
    void drawSeats();
    void updateConfirmation();
    UiSession& session_;
    UiCallbacks callbacks_;
    QComboBox* showtimeBox_;
    QComboBox* customerBox_;
    QComboBox* methodBox_;
    QGridLayout* seatGrid_;
    QLabel* roomLabel_;
    QLabel* confirmationLabel_;
    QPushButton* bookButton_;
    QString selectedSeat_;
};

class CustomerPage : public QWidget {
public:
    CustomerPage(UiSession& session, UiCallbacks callbacks, QWidget* parent = nullptr);
    void refresh();
private:
    UiSession& session_;
    UiCallbacks callbacks_;
    QLineEdit* idEdit_;
    QLineEdit* nameEdit_;
    QVBoxLayout* customerList_;
};

class TicketPage : public QWidget {
public:
    TicketPage(UiSession& session, UiCallbacks callbacks, QWidget* parent = nullptr);
    void refresh();
private:
    UiSession& session_;
    UiCallbacks callbacks_;
    QVBoxLayout* ticketList_;
};

class PaymentPage : public QWidget {
public:
    PaymentPage(UiSession& session, UiCallbacks callbacks, QWidget* parent = nullptr);
    void refresh();
private:
    UiSession& session_;
    UiCallbacks callbacks_;
    QComboBox* ticketBox_;
    QComboBox* methodBox_;
    QPushButton* payButton_;
};
