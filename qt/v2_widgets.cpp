#include "v2_widgets.h"
#include "qrcodegen.hpp"

#include <QFrame>
#include <QDir>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>

QString vnd(long long amount) {
    return QString("%1 ₫").arg(QString::number(amount).replace(
        QRegularExpression("(?<=\\d)(?=(\\d{3})+$)"), "."));
}

QString posterFile(const QString& assets, const MovieDetails& movie) {
    return QDir(assets).filePath(QString::fromStdString(movie.posterAsset));
}

MovieBanner::MovieBanner(const std::vector<const MovieDetails*>& movies,
                         const QString& assets, QWidget* parent)
    : QWidget(parent), movies_(movies), assets_(assets) {
    setObjectName("movieBanner");
    setFixedHeight(285);
    background_ = new QLabel(this);
    background_->setObjectName("bannerBackground");
    background_->setAlignment(Qt::AlignCenter);
    background_->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto* blur = new QGraphicsBlurEffect(background_);
    blur->setBlurRadius(28);
    background_->setGraphicsEffect(blur);
    auto* veil = new QWidget(this);
    veil->setObjectName("bannerVeil");
    veil->setAttribute(Qt::WA_TransparentForMouseEvents);
    foreground_ = new QWidget(this);
    foreground_->setObjectName("bannerForeground");
    auto* row = new QHBoxLayout(foreground_);
    row->setContentsMargins(20, 19, 20, 19);
    row->setSpacing(18);
    previous_ = new QPushButton("‹");
    previous_->setObjectName("bannerPrev");
    previous_->setFixedWidth(38);
    row->addWidget(previous_);
    poster_ = new QLabel;
    poster_->setObjectName("bannerPoster");
    poster_->setFixedSize(150, 225);
    poster_->setAlignment(Qt::AlignCenter);
    row->addWidget(poster_);
    auto* copy = new QWidget;
    auto* words = new QVBoxLayout(copy);
    words->setContentsMargins(0, 0, 0, 0);
    words->setSpacing(12);
    auto* eyebrow = new QLabel("PHIM NỔI BẬT · OOP CINEMA");
    eyebrow->setObjectName("bannerEyebrow");
    words->addWidget(eyebrow);
    title_ = new QLabel;
    title_->setObjectName("bannerTitle");
    title_->setWordWrap(true);
    words->addWidget(title_);
    facts_ = new QLabel;
    facts_->setObjectName("bannerFacts");
    facts_->setWordWrap(true);
    words->addWidget(facts_);
    auto* actions = new QHBoxLayout;
    auto* book = new QPushButton("Đặt vé ngay  →");
    book->setObjectName("bannerBook");
    book->setProperty("role", "primary");
    auto* details = new QPushButton("Xem chi tiết");
    details->setObjectName("bannerDetails");
    actions->addWidget(book);
    actions->addWidget(details);
    actions->addStretch();
    words->addLayout(actions);
    words->addStretch();
    row->addWidget(copy, 1);
    next_ = new QPushButton("›");
    next_->setObjectName("bannerNext");
    next_->setFixedWidth(38);
    row->addWidget(next_);
    background_->lower();
    veil->raise();
    foreground_->raise();
    connect(previous_, &QPushButton::clicked, this,
            [this] { showIndex((index_ + int(movies_.size()) - 1) % int(movies_.size())); });
    connect(next_, &QPushButton::clicked, this,
            [this] { showIndex((index_ + 1) % int(movies_.size())); });
    connect(book, &QPushButton::clicked, this,
            [this] { if (onBook_) onBook_(currentMovieId()); });
    connect(details, &QPushButton::clicked, this,
            [this] { if (onDetails_) onDetails_(currentMovieId()); });
    previous_->setVisible(movies_.size() > 1);
    next_->setVisible(movies_.size() > 1);
    showIndex(0);
}

int MovieBanner::currentMovieId() const {
    return movies_.empty() ? 0 : movies_[index_]->movieId;
}

void MovieBanner::showIndex(int index) {
    if (movies_.empty()) return;
    index_ = index;
    const MovieDetails& movie = *movies_[index_];
    const QPixmap art(posterFile(assets_, movie));
    poster_->setPixmap(art.scaled(poster_->size(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation));
    poster_->setAccessibleName("Poster phim " + QString::fromStdString(movie.title));
    title_->setText(QString::fromStdString(movie.title));
    facts_->setText(QString("%1  ·  %2 phút  ·  %3")
        .arg(QString::fromStdString(movie.genre)).arg(movie.durationMinutes)
        .arg(movie.ageRating.empty() ? "Chưa phân loại" :
             QString::fromStdString(movie.ageRating)));
    refreshBackground();
    auto* opacity = new QGraphicsOpacityEffect(foreground_);
    foreground_->setGraphicsEffect(opacity);
    auto* fade = new QPropertyAnimation(opacity, "opacity", foreground_);
    fade->setDuration(220);
    fade->setStartValue(0.35);
    fade->setEndValue(1.0);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void MovieBanner::refreshBackground() {
    if (movies_.empty()) return;
    const QPixmap art(posterFile(assets_, *movies_[index_]));
    if (!art.isNull() && !background_->size().isEmpty())
        background_->setPixmap(art.scaled(background_->size(),
            Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void MovieBanner::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    background_->setGeometry(rect());
    if (auto* veil = findChild<QWidget*>("bannerVeil")) veil->setGeometry(rect());
    foreground_->setGeometry(rect());
    refreshBackground();
}

namespace {
QLabel* textLabel(const QString& text, const QString& kind = {}) {
    auto* label = new QLabel(text);
    if (!kind.isEmpty()) label->setProperty("kind", kind);
    label->setWordWrap(true);
    return label;
}

QWidget* posterCard(const MovieDetails& movie, const QString& assets,
                    const std::function<void(int)>& onMovie) {
    auto* card = new QFrame;
    card->setObjectName("movieCard");
    card->setMinimumWidth(162);
    auto* box = new QVBoxLayout(card);
    box->setContentsMargins(0, 0, 0, 14);
    box->setSpacing(9);
    auto* image = new QLabel;
    image->setObjectName("posterImage");
    image->setFixedHeight(230);
    image->setAlignment(Qt::AlignCenter);
    QPixmap poster(posterFile(assets, movie));
    if (!poster.isNull()) image->setPixmap(poster.scaled(154, 230,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    else image->setText("Không mở được poster");
    box->addWidget(image);
    auto* title = textLabel(QString::fromStdString(movie.title), "cardTitle");
    title->setMinimumHeight(42);
    box->addWidget(title);
    auto* meta = textLabel(QString("%1 phút  •  %2").arg(movie.durationMinutes)
                           .arg(QString::fromStdString(movie.ageRating).isEmpty() ?
                                "Chưa phân loại" : QString::fromStdString(movie.ageRating)),
                           "muted");
    box->addWidget(meta);
    auto* button = new QPushButton("Xem phim & đặt vé  →");
    button->setObjectName(QString("movie-%1").arg(movie.movieId));
    button->setProperty("role", "primary");
    QObject::connect(button, &QPushButton::clicked, card, [onMovie, id=movie.movieId] {
        if (onMovie) onMovie(id);
    });
    box->addWidget(button);
    return card;
}
}

MovieGallery::MovieGallery(const V2Session& session, const QString& assets, QWidget* parent)
    : QWidget(parent), session_(session), assets_(assets) {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(20);
    auto* heading = textLabel("PHIM CÓ LỊCH CHIẾU", "eyebrow");
    root->addWidget(heading);
    root->addWidget(textLabel("Chọn câu chuyện cho buổi hẹn của bạn", "sectionTitle"));
    auto* search = new QLineEdit;
    search->setPlaceholderText("Tìm phim theo tên…");
    search->setClearButtonEnabled(true);
    root->addWidget(search);
    auto* gridHost = new QWidget;
    auto* grid = new QGridLayout(gridHost);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(16);
    root->addWidget(gridHost);
    root->addStretch();

    auto redraw = [this, grid](const QString& query) {
        while (QLayoutItem* item = grid->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        int index = 0;
        for (const auto* movie : session_.featuredMovies()) {
            if (!QString::fromStdString(movie->title).contains(query, Qt::CaseInsensitive)) continue;
            grid->addWidget(posterCard(*movie, assets_, [this](int id) {
                if (onMovie_) onMovie_(id);
            }), index / 4, index % 4);
            ++index;
        }
        if (index == 0) grid->addWidget(textLabel("Không tìm thấy phim phù hợp.", "muted"), 0, 0);
    };
    QObject::connect(search, &QLineEdit::textChanged, this, redraw);
    redraw({});
}

void MovieGallery::setOnMovie(std::function<void(int)> callback) {
    onMovie_ = std::move(callback);
}

SeatMapWidget::SeatMapWidget(const V2Session& session, int showtimeId, QWidget* parent)
    : QWidget(parent), session_(session), showtimeId_(showtimeId) {
    setObjectName("seatMap");
    auto* root = new QVBoxLayout(this);
    root->setSpacing(16);
    auto* screen = textLabel("MÀN HÌNH", "screen");
    screen->setAlignment(Qt::AlignCenter);
    root->addWidget(screen);
    auto* host = new QWidget;
    grid_ = new QGridLayout(host);
    grid_->setSpacing(5);
    grid_->setContentsMargins(0, 0, 0, 0);
    root->addWidget(host, 0, Qt::AlignHCenter);
    auto* legend = textLabel(
        "<span style='color:#d69b69'>■</span> Trống &nbsp; "
        "<span style='color:#cf641e'>■</span> VIP &nbsp; "
        "<span style='color:#a6a09a'>■</span> Đang giữ &nbsp; "
        "<span style='color:#bf8584'>■</span> Đã đặt &nbsp; "
        "<span style='color:#ed6b21'>■</span> Đang chọn", "seatLegend");
    legend->setAlignment(Qt::AlignCenter);
    root->addWidget(legend);
    refresh();
}

void SeatMapWidget::refresh() {
    while (QLayoutItem* item = grid_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    const Showtime* show = session_.cinema().schedule().findShowtime(showtimeId_);
    const RoomSeatLayout* layout = show ? session_.cinema().layout(show->getRoomId()) : nullptr;
    if (!layout) return;
    grid_->setColumnMinimumWidth(layout->aisleAfter() + 2, 25);
    for (const auto& seat : layout->positions()) {
        auto* button = new QPushButton(QString::number(seat.number));
        button->setObjectName("seat-" + QString::fromStdString(seat.seatId));
        button->setFixedSize(39, 34);
        button->setToolTip(QString::fromStdString(seat.seatId) +
                           (seat.category == SeatCategory::VIP ? " · VIP" : " · Standard"));
        const SeatStatus status = session_.seatStatus(showtimeId_, seat.seatId);
        QString state = status == SeatStatus::Held ? "held" :
                        status == SeatStatus::Booked ? "booked" :
                        seat.category == SeatCategory::VIP ? "vip" : "available";
        if (selected_.count(seat.seatId)) state = "selected";
        button->setProperty("seatState", state);
        button->setEnabled(status == SeatStatus::Available);
        QObject::connect(button, &QPushButton::clicked, this, [this, id=seat.seatId] {
            if (selected_.count(id)) selected_.erase(id);
            else if (selected_.size() < 8) selected_.insert(id);
            refresh();
            if (onChange_) onChange_();
        });
        grid_->addWidget(button, seat.row - 'A', seat.column + 1);
    }
    for (int row = 0; row < layout->rowCount(); ++row) {
        auto* label = textLabel(QString(QChar('A' + row)), "rowLabel");
        grid_->addWidget(label, row, 0);
    }
}

TicketCard::TicketCard(const V2Session& session, const CinemaOrder& order, QWidget* parent)
    : QWidget(parent) {
    setObjectName("ticketCard");
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(22, 20, 22, 20);
    root->setSpacing(26);
    auto* details = new QWidget;
    auto* words = new QVBoxLayout(details);
    words->setSpacing(10);
    const MovieDetails* movie = session.movies().find(order.movieId);
    const Showtime* show = session.cinema().schedule().findShowtime(order.showtimeId);
    const CinemaBranch* branch = session.cinema().branches().find(order.branchId);
    QStringList seats;
    for (const auto& seat : order.seatIds) seats << seat;
    words->addWidget(textLabel("VÉ ĐIỆN TỬ  ·  OOP CINEMA", "eyebrow"));
    words->addWidget(textLabel(movie ? QString::fromStdString(movie->title) : "Phim không tìm thấy",
                              "sectionTitle"));
    words->addWidget(textLabel(QString("%1  •  %2")
        .arg(branch ? QString::fromStdString(branch->name) : "Rạp không tìm thấy")
        .arg(show ? QString::fromStdString(show->getStartTime()) : "Suất không tìm thấy")));
    words->addWidget(textLabel("Ghế: " + seats.join(", ") + "   •   Người mua: " + order.buyer));
    words->addWidget(textLabel("Mã đặt vé: " + order.code, "bookingCode"));
    words->addWidget(textLabel("Tổng đơn: " +
        vnd(order.ticketTotal + order.snackTotal - order.discount), "price"));
    words->addStretch();
    root->addWidget(details, 1);
    auto* qrColumn = new QWidget;
    auto* qrLayout = new QVBoxLayout(qrColumn);
    bool active = false;
    for (int id : order.ticketIds) for (const Ticket& ticket : session.tickets())
        if (ticket.getId() == id && ticket.getStatus() != TicketStatus::Cancelled)
            active = true;
    if (active) {
        const QString payload = QString("OOP-CINEMA|%1|%2|%3")
            .arg(order.code).arg(order.movieId).arg(order.showtimeId);
        const auto qr = qrcodegen::QrCode::encodeText(payload.toUtf8().constData(),
                                                     qrcodegen::QrCode::Ecc::MEDIUM);
        const int pixel = 5, quiet = 4;
        const int size = (qr.getSize() + quiet * 2) * pixel;
        QImage image(size, size, QImage::Format_RGB32);
        image.fill(Qt::white);
        for (int y = 0; y < qr.getSize(); ++y)
            for (int x = 0; x < qr.getSize(); ++x)
                if (qr.getModule(x, y))
                    for (int dy = 0; dy < pixel; ++dy)
                        for (int dx = 0; dx < pixel; ++dx)
                            image.setPixel((x + quiet) * pixel + dx,
                                           (y + quiet) * pixel + dy, qRgb(0, 0, 0));
        auto* qrImage = new QLabel;
        qrImage->setPixmap(QPixmap::fromImage(image));
        qrImage->setFixedSize(size, size);
        qrImage->setAccessibleName("QR nội bộ của đơn " + order.code);
        qrLayout->addWidget(qrImage, 0, Qt::AlignCenter);
        qrLayout->addWidget(textLabel("QR nhận diện đơn nội bộ", "muted"));
    } else {
        qrLayout->addWidget(textLabel("VÉ ĐÃ HỦY\nMã QR không còn hiệu lực", "muted"));
    }
    root->addWidget(qrColumn, 0, Qt::AlignTop);
}
