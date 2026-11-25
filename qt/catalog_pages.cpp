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
    connect(discover, &QPushButton::clicked, this, [this] { callbacks_.chooseMovie(0); });
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
    if (session_.movies().getMovies().empty())
        moviesLayout_->addWidget(mutedLabel("Chưa có phim trong dữ liệu hiện tại.", this));
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
        name->setWordWrap(true);
        details->addWidget(name);
        details->addWidget(mutedLabel(QString("%1 phút  •  %2  •  từ %3")
            .arg(movie.getDurationMinutes())
            .arg(QString::fromStdString(movie.getGenre()))
            .arg(vnd(movie.getBasePriceVnd())), frame));
        row->addLayout(details, 1);
        auto* button = new QPushButton("Chọn suất  →", frame);
        button->setObjectName("movieButton");
        button->setProperty("movieId", movie.getId());
        const int movieId = movie.getId();
        connect(button, &QPushButton::clicked, this,
                [this, movieId] { callbacks_.chooseMovie(movieId); });
        row->addWidget(button);
        moviesLayout_->addWidget(frame);
    }
}

ShowtimePage::ShowtimePage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("showtimePage");
    auto* root = pageLayout(this, "Lịch chiếu", "Các suất chiếu và mức giá của từng phim.");
    auto* filterRow = new QHBoxLayout;
    filterLabel_ = mutedLabel("Tất cả phim", this);
    filterLabel_->setObjectName("movieFilterLabel");
    filterRow->addWidget(filterLabel_);
    filterRow->addStretch();
    clearFilter_ = new QPushButton("Xem tất cả", this);
    clearFilter_->setObjectName("clearMovieFilter");
    connect(clearFilter_, &QPushButton::clicked, this, [this] { setMovieFilter(0); });
    filterRow->addWidget(clearFilter_);
    root->addLayout(filterRow);
    auto* list = new QWidget(this);
    showsLayout_ = new QVBoxLayout(list);
    showsLayout_->setContentsMargins(0, 0, 0, 0);
    showsLayout_->setSpacing(12);
    root->addWidget(list);
    root->addStretch();
    refresh();
}

void ShowtimePage::setMovieFilter(int movieId) {
    movieFilter_ = movieId;
    refresh();
}

void ShowtimePage::refresh() {
    clearLayout(showsLayout_);
    const Movie* filteredMovie = session_.movies().findMovie(movieFilter_);
    if (movieFilter_ && !filteredMovie) movieFilter_ = 0;
    filterLabel_->setText(movieFilter_ ?
        QString("Suất chiếu của: %1").arg(QString::fromStdString(filteredMovie->getTitle())) :
        "Tất cả phim");
    clearFilter_->setVisible(movieFilter_ != 0);
    int visibleCount = 0;
    for (const Showtime& show : session_.schedule().getShowtimes()) {
        if (movieFilter_ && show.getMovieId() != movieFilter_) continue;
        ++visibleCount;
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
        name->setWordWrap(true);
        details->addWidget(name);
        details->addWidget(mutedLabel(QString("%1  •  Phòng %2  •  %3")
            .arg(QString::fromStdString(show.getStartTime()).left(10))
            .arg(show.getRoomId()).arg(vnd(show.getPriceVnd())), frame));
        row->addLayout(details, 1);
        auto* button = new QPushButton("Chọn ghế  →", frame);
        button->setObjectName("showtimeButton");
        button->setProperty("showtimeId", show.getId());
        const int showtimeId = show.getId();
        connect(button, &QPushButton::clicked, this,
                [this, showtimeId] { callbacks_.chooseShowtime(showtimeId); });
        row->addWidget(button);
        showsLayout_->addWidget(frame);
    }
    if (visibleCount == 0)
        showsLayout_->addWidget(mutedLabel("Chưa có suất chiếu cho phim này.", this));
}
