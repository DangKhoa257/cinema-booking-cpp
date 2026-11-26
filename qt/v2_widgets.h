#pragma once

#include "v2_session.h"
#include <QWidget>
#include <functional>
#include <set>

class QGridLayout;
class QLabel;

QString vnd(long long amount);
QString posterFile(const QString& assets, int movieId);

class MovieGallery : public QWidget {
public:
    MovieGallery(const V2Session& session, const QString& assets, QWidget* parent = nullptr);
    void setOnMovie(std::function<void(int)> callback);
private:
    const V2Session& session_;
    QString assets_;
    std::function<void(int)> onMovie_;
};

class SeatMapWidget : public QWidget {
public:
    SeatMapWidget(const V2Session& session, int showtimeId, QWidget* parent = nullptr);
    const std::set<std::string>& selected() const { return selected_; }
    void setSelected(const std::set<std::string>& seats) { selected_ = seats; refresh(); }
    void setOnChange(std::function<void()> callback) { onChange_ = std::move(callback); }
    void refresh();
private:
    const V2Session& session_;
    int showtimeId_;
    QGridLayout* grid_ = nullptr;
    std::set<std::string> selected_;
    std::function<void()> onChange_;
};

class TicketCard : public QWidget {
public:
    TicketCard(const V2Session& session, const CinemaOrder& order,
               QWidget* parent = nullptr);
};
