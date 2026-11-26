#pragma once

#include "v2_session.h"
#include <QMainWindow>
#include <memory>
#include <set>

class QComboBox;
class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class V2Window : public QMainWindow {
public:
    V2Window(const QString& movieFile, const QString& cinemaFile,
             const QString& assets, const QString& storageDirectory,
             QWidget* parent = nullptr);
    const V2Session& session() const { return session_; }
private:
    void showHome();
    void showMovie(int movieId);
    void showSeats(int showtimeId);
    void showSnacks();
    void showCheckout();
    void showTicket(const QString& code);
    void showOrders();
    void setContent(QWidget* page, const QString& section);
    bool saveChanges();
    void closeEvent(QCloseEvent* event) override;

    V2Session session_;
    QString assets_;
    QWidget* content_ = nullptr;
    QVBoxLayout* contentLayout_ = nullptr;
    QLabel* status_ = nullptr;
    QPushButton* homeNav_ = nullptr;
    QPushButton* ticketsNav_ = nullptr;
    int movieId_ = 0;
    int branchId_ = 0;
    int showtimeId_ = 0;
    std::set<std::string> seats_;
    std::vector<int> quantities_;
    QString voucher_;
};
