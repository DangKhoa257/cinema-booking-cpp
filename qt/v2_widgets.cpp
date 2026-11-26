#include "v2_widgets.h"
#include "qrcodegen.hpp"

#include <QFrame>
#include <QDir>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QVBoxLayout>

QString vnd(long long amount) {
    return QString("%1 ₫").arg(QString::number(amount).replace(
        QRegularExpression("(?<=\\d)(?=(\\d{3})+$)"), "."));
}

QString posterFile(const QString& assets, int movieId) {
    return QDir(assets).filePath(QString("poster-%1.png").arg(movieId));
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
    QPixmap poster(posterFile(assets, movie.movieId));
    if (!poster.isNull()) image->setPixmap(poster.scaled(190, 230,
                                    Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation));
    else image->setText("OOP CINEMA\nHình minh họa");
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
    auto* heading = textLabel("PHIM ĐANG CHIẾU", "eyebrow");
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
        for (const auto& movie : session_.movies().all()) {
            if (!QString::fromStdString(movie.title).contains(query, Qt::CaseInsensitive)) continue;
            grid->addWidget(posterCard(movie, assets_, [this](int id) {
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
        "<span style='color:#66b7aa'>■</span> Trống &nbsp; "
        "<span style='color:#deb478'>■</span> VIP &nbsp; "
        "<span style='color:#7b8195'>■</span> Đang giữ &nbsp; "
        "<span style='color:#765168'>■</span> Đã đặt &nbsp; "
        "<span style='color:#f4757a'>■</span> Đang chọn", "seatLegend");
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
