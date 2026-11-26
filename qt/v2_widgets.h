#pragma once

#include "v2_session.h"
#include <QWidget>
#include <functional>
#include <set>

class QGridLayout;
class QLabel;
class QPushButton;
class QResizeEvent;

// Banner dùng poster dọc của chính phim, nền mờ chỉ là bản phóng của cùng ảnh.
class MovieBanner : public QWidget {
public:
    MovieBanner(const std::vector<const MovieDetails*>& movies, const QString& assets,
                QWidget* parent = nullptr);
    int currentMovieId() const;
    void setOnBook(std::function<void(int)> callback) { onBook_ = std::move(callback); }
    void setOnDetails(std::function<void(int)> callback) { onDetails_ = std::move(callback); }
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    void showIndex(int index);
    void refreshBackground();
    std::vector<const MovieDetails*> movies_;
    QString assets_;
    int index_ = 0;
    QLabel* background_ = nullptr;
    QLabel* poster_ = nullptr;
    QLabel* title_ = nullptr;
    QLabel* facts_ = nullptr;
    QWidget* foreground_ = nullptr;
    QPushButton* previous_ = nullptr;
    QPushButton* next_ = nullptr;
    std::function<void(int)> onBook_;
    std::function<void(int)> onDetails_;
};

QString vnd(long long amount);
QString posterFile(const QString& assets, const MovieDetails& movie);

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
