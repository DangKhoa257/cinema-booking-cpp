#include "ui_pages.h"
#include "ui_helpers.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

HomePage::HomePage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("homePage");
    auto* root = pageLayout(this, "Khám phá điện ảnh", "Chọn phim yêu thích và bắt đầu hành trình đặt vé.");
    auto* hero = card(this);
    hero->setObjectName("hero");
    auto* heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(30, 24, 30, 24);
    auto* badge = mutedLabel("CINEMA BOOKING  •  BẢN DEMO", hero);
    heroLayout->addWidget(badge);
    auto* headline = new QLabel("Chỗ ngồi đẹp. Trải nghiệm hay.", hero);
    headline->setObjectName("heroTitle");
    headline->setWordWrap(true);
    heroLayout->addWidget(headline);
    heroLayout->addWidget(mutedLabel("Xem lịch chiếu, chọn ghế và quản lý vé trong một nơi.", hero));
    auto* discover = new QPushButton("Xem suất chiếu  →", hero);
    discover->setObjectName("primary");
    discover->setFixedWidth(190);
    connect(discover, &QPushButton::clicked, this, [this] { callbacks_.navigate(1); });
    heroLayout->addWidget(discover);
    root->addWidget(hero);
    auto* section = new QLabel("ĐANG CHIẾU", this);
    section->setObjectName("sectionTitle");
    root->addWidget(section);
    auto* list = new QWidget(this);
    moviesLayout_ = new QVBoxLayout(list);
    moviesLayout_->setContentsMargins(0, 0, 0, 0);
    moviesLayout_->setSpacing(12);
    root->addWidget(list);
    root->addStretch();
    refresh();
}

void HomePage::refresh() {
    clearLayout(moviesLayout_);
    for (const Movie& movie : session_.movies().getMovies()) {
        auto* frame = card(this);
        auto* row = new QHBoxLayout(frame);
        row->setContentsMargins(22, 16, 22, 16);
        auto* accent = new QLabel(QString::number(movie.getId()).rightJustified(2, '0'), frame);
        accent->setObjectName("poster");
        accent->setAlignment(Qt::AlignCenter);
        accent->setFixedSize(64, 80);
        row->addWidget(accent);
        auto* details = new QVBoxLayout;
        auto* name = new QLabel(QString::fromStdString(movie.getTitle()), frame);
        name->setObjectName("cardTitle");
        details->addWidget(name);
        details->addWidget(mutedLabel(QString("%1 phút  •  %2  •  từ %3 đ")
            .arg(movie.getDurationMinutes())
            .arg(QString::fromStdString(movie.getGenre()))
            .arg(movie.getBasePriceVnd()), frame));
        row->addLayout(details, 1);
        auto* button = new QPushButton("Chọn suất  →", frame);
        connect(button, &QPushButton::clicked, this, [this] { callbacks_.navigate(1); });
        row->addWidget(button);
        moviesLayout_->addWidget(frame);
    }
}

ShowtimePage::ShowtimePage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("showtimePage");
    auto* root = pageLayout(this, "Lịch chiếu", "Các suất chiếu và mức giá của từng phim.");
    auto* list = new QWidget(this);
    showsLayout_ = new QVBoxLayout(list);
    showsLayout_->setContentsMargins(0, 0, 0, 0);
    showsLayout_->setSpacing(12);
    root->addWidget(list);
    root->addStretch();
    refresh();
}

void ShowtimePage::refresh() {
    clearLayout(showsLayout_);
    for (const Showtime& show : session_.schedule().getShowtimes()) {
        auto* frame = card(this);
        auto* row = new QHBoxLayout(frame);
        row->setContentsMargins(22, 18, 22, 18);
        auto* time = new QLabel(QString::fromStdString(show.getStartTime()).mid(11), frame);
        time->setObjectName("showTime");
        time->setFixedWidth(85);
        row->addWidget(time);
        auto* details = new QVBoxLayout;
        const Movie* movie = session_.movies().findMovie(show.getMovieId());
        auto* name = new QLabel(movie ? QString::fromStdString(movie->getTitle()) : "Không có phim", frame);
        name->setObjectName("cardTitle");
        details->addWidget(name);
        details->addWidget(mutedLabel(QString("%1  •  Phòng %2  •  %3 đ")
            .arg(QString::fromStdString(show.getStartTime()).left(10))
            .arg(show.getRoomId()).arg(show.getPriceVnd()), frame));
        row->addLayout(details, 1);
        auto* button = new QPushButton("Chọn ghế  →", frame);
        connect(button, &QPushButton::clicked, this, [this] { callbacks_.navigate(2); });
        row->addWidget(button);
        showsLayout_->addWidget(frame);
    }
}
