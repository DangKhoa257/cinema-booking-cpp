#pragma once

#include "ui_pages.h"
#include "storage_bar.h"
#include <QMainWindow>
#include <vector>

class QLabel;
class QPushButton;
class QStackedWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    explicit MainWindow(const QString& dataDirectory, QWidget* parent = nullptr);
    void showPage(int index);
    int currentPage() const;
    bool saveData();
    bool loadData();
    QString dataDirectory() const { return dataDirectory_; }

private:
    void refreshAll();
    void chooseDirectory();
    void closeEvent(QCloseEvent* event) override;
    UiSession session_;
    HomePage* home_;
    ShowtimePage* showtimes_;
    SeatMapPage* seats_;
    CustomerPage* customers_;
    TicketPage* tickets_;
    PaymentPage* payments_;
    QStackedWidget* pages_;
    QLabel* toast_;
    StorageBar* storageBar_;
    QString dataDirectory_;
    bool loadFailed_ = false;
    std::vector<QPushButton*> navigation_;
};
