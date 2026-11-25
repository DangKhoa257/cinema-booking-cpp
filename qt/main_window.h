#pragma once

#include "ui_pages.h"
#include <QMainWindow>
#include <vector>

class QLabel;
class QPushButton;
class QStackedWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void showPage(int index);
    int currentPage() const;

private:
    void refreshAll();
    UiSession session_;
    HomePage* home_;
    ShowtimePage* showtimes_;
    SeatMapPage* seats_;
    CustomerPage* customers_;
    TicketPage* tickets_;
    PaymentPage* payments_;
    QStackedWidget* pages_;
    QLabel* toast_;
    std::vector<QPushButton*> navigation_;
};
