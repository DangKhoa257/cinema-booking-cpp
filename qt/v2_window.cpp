#include "v2_window.h"
#include "v2_widgets.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QDate>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QLabel* label(const QString& text, const QString& kind = {}) {
    auto* result = new QLabel(text);
    result->setWordWrap(true);
    if (!kind.isEmpty()) result->setProperty("kind", kind);
    return result;
}
QPushButton* action(const QString& text, bool primary = false) {
    auto* result = new QPushButton(text);
    if (primary) result->setProperty("role", "primary");
    return result;
}
QFrame* panel() {
    auto* result = new QFrame;
    result->setObjectName("panel");
    return result;
}
QString titleOf(const MovieDetails* movie) {
    return movie ? QString::fromStdString(movie->title) : "Phim không tìm thấy";
}
QString statusOf(const V2Session& session, const CinemaOrder& order) {
    bool pending = false, paid = false;
    for (int id : order.ticketIds) for (const Ticket& ticket : session.tickets())
        if (ticket.getId() == id) {
            pending |= ticket.getStatus() == TicketStatus::Pending;
            paid |= ticket.getStatus() == TicketStatus::Paid;
        }
    return pending ? "Đang chờ thanh toán" : paid ? "Đã thanh toán demo" : "Đã hủy";
}
QString joinSeats(const std::set<std::string>& seats) {
    QStringList names;
    for (const auto& seat : seats) names << QString::fromStdString(seat);
    return names.join(", ");
}
QString longDate(const std::string& value) {
    return QDate::fromString(QString::fromStdString(value.substr(0, 10)), "yyyy-MM-dd")
        .toString("dd/MM/yyyy") + " • " + QString::fromStdString(value.substr(11, 5));
}
}

V2Window::V2Window(const QString& movieFile, const QString& cinemaFile,
                   const QString& assets, const QString& storageDirectory, QWidget* parent)
    : QMainWindow(parent), session_(movieFile, cinemaFile, storageDirectory), assets_(assets) {
    setWindowTitle("OOP Cinema 2.0 — Đặt vé xem phim");
    setMinimumSize(860, 620);
    resize(1120, 760);
    auto* shell = new QWidget;
    shell->setObjectName("shell");
    auto* outer = new QVBoxLayout(shell);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* header = new QFrame;
    header->setObjectName("topBar");
    auto* bar = new QHBoxLayout(header);
    bar->setContentsMargins(32, 13, 32, 13);
    bar->setSpacing(24);
    auto* brand = label("OOP <font color='#e85c17'>CINEMA</font> 2.0", "brand");
    bar->addWidget(brand);
    homeNav_ = action("Danh mục phim");
    ticketsNav_ = action("Vé của tôi");
    homeNav_->setProperty("role", "nav");
    ticketsNav_->setProperty("role", "nav");
    bar->addWidget(homeNav_);
    bar->addWidget(ticketsNav_);
    bar->addStretch();
    outer->addWidget(header);
    connect(homeNav_, &QPushButton::clicked, this, [this] { showHome(); });
    connect(ticketsNav_, &QPushButton::clicked, this, [this] { showOrders(); });

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setObjectName("pageScroll");
    content_ = new QWidget;
    content_->setObjectName("pageBody");
    contentLayout_ = new QVBoxLayout(content_);
    contentLayout_->setContentsMargins(28, 28, 28, 52);
    contentLayout_->setSpacing(0);
    scroll->setWidget(content_);
    outer->addWidget(scroll, 1);
    status_ = label("", "status");
    status_->setMinimumHeight(30);
    outer->addWidget(status_);
    setCentralWidget(shell);

    setStyleSheet(R"(
        QWidget#shell, QWidget#pageBody, QScrollArea#pageScroll {background:#fff8f0;color:#2d211d;}
        QFrame#topBar {background:#fff3e7;border-bottom:1px solid #edd1bb;}
        QLabel[kind="brand"] {font-size:22px;font-weight:900;letter-spacing:2px;color:#2d211d;}
        QPushButton {background:#fff8f0;color:#593823;border:1px solid #ddb894;border-radius:9px;padding:10px 16px;font-weight:600;}
        QPushButton:hover {background:#ffead7;border-color:#df7d36;}
        QPushButton:disabled {color:#a99a8c;background:#f5eee7;border-color:#e4d9cd;}
        QPushButton[role="primary"] {background:#e85c17;color:white;border-color:#e85c17;font-weight:800;}
        QPushButton[role="primary"]:hover {background:#f47724;}
        QPushButton[role="primary"]:disabled {background:#e4b798;color:#fff5ee;border-color:#e4b798;}
        QPushButton[role="nav"] {background:transparent;border:0;border-radius:0;color:#5e5148;padding:12px 8px;}
        QPushButton[role="navActive"] {background:transparent;border:0;border-bottom:2px solid #e85c17;border-radius:0;color:#c84910;padding:12px 8px;}
        QLabel[kind="eyebrow"] {color:#ae541f;font-size:11px;font-weight:800;letter-spacing:2px;}
        QLabel[kind="sectionTitle"] {font-size:30px;font-weight:800;color:#2d211d;}
        QLabel[kind="muted"], QLabel[kind="seatLegend"] {color:#76655b;}
        QLabel[kind="price"] {font-size:20px;font-weight:800;color:#b94612;}
        QLabel[kind="bookingCode"] {font-size:21px;font-weight:900;color:#a64216;}
        QLabel[kind="orderMovie"] {font-size:19px;font-weight:800;color:#2d211d;}
        QLabel[kind="orderFact"] {font-size:14px;color:#624735;}
        QLabel[kind="status"] {background:#fcebd9;color:#69452c;padding:5px 28px;font-size:12px;}
        QLabel[kind="screen"] {background:#f7d9ba;color:#8e461e;border-bottom:3px solid #d9702e;border-radius:4px;padding:11px;letter-spacing:5px;font-weight:800;}
        QLabel[kind="rowLabel"] {color:#704b34;font-weight:800;}
        QFrame#panel, QWidget#ticketCard {background:#fffdf9;border:1px solid #ebd4bf;border-radius:17px;}
        QFrame#movieCard {background:#fffdf9;border:1px solid #ebd4bf;border-radius:13px;}
        QFrame#snackCard {background:#fffdf9;border:1px solid #ebd4bf;border-radius:14px;}
        QFrame#snackCard:hover {background:#fff8ef;border-color:#d8894f;}
        QLabel[kind="snackName"] {font-size:17px;font-weight:800;color:#2d211d;}
        QLabel[kind="snackContents"] {font-size:13px;color:#76655b;}
        QLabel[kind="snackPrice"] {font-size:17px;font-weight:800;color:#bd4b15;}
        QLabel[kind="snackCount"] {font-size:16px;font-weight:800;color:#2d211d;}
        QPushButton[role="quantity"] {font-size:20px;font-weight:800;padding:0;background:#fff4e8;color:#a74312;}
        QPushButton[role="quantity"]:hover {background:#ffddb9;}
        QLabel#posterImage {background:#f3e4d4;border-radius:9px;}
        QLabel[kind="cardTitle"] {font-size:16px;font-weight:800;color:#2d211d;margin-left:9px;margin-right:9px;}
        QLabel[kind="muted"] {margin-left:3px;}
        QWidget#movieBanner {background:#f5e2cf;border:1px solid #e8bf9d;border-radius:17px;}
        QWidget#bannerVeil {background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,247,237,140),stop:0.55 rgba(255,247,237,223),stop:1 rgba(255,247,237,185));border-radius:17px;}
        QWidget#bannerForeground {background:transparent;}
        QLabel#bannerEyebrow {color:#a84b17;font-size:11px;font-weight:800;letter-spacing:1px;}
        QLabel#bannerTitle {font-size:27px;font-weight:900;color:#31211a;}
        QLabel#bannerFacts {font-size:14px;color:#6b4a36;}
        QLabel#bannerPoster {background:transparent;}
        QPushButton#bannerPrev, QPushButton#bannerNext {font-size:26px;padding:0;background:#fffaf4;}
        QLineEdit, QComboBox {background:#fffdf9;color:#33241d;border:1px solid #d9b99f;border-radius:8px;padding:9px;min-height:20px;}
        QComboBox QAbstractItemView {background:#fffaf4;color:#33241d;selection-background-color:#ffddbd;}
        QPushButton[seatState="available"] {background:#ffe4cb;border-color:#d69b69;color:#563a29;padding:0;}
        QPushButton[seatState="vip"] {background:#f6b269;border-color:#cf641e;color:#4e240d;padding:0;}
        QPushButton[seatState="selected"] {background:#ed6b21;border-color:#c94d12;color:white;padding:0;}
        QPushButton[seatState="held"] {background:#c5c0bb;border-color:#a6a09a;color:#564f48;padding:0;}
        QPushButton[seatState="booked"] {background:#e7bbbb;border-color:#bf8584;color:#6f4140;padding:0;}
        QRadioButton {spacing:8px;color:#3a2920;}
    )");
    if (!session_.ready()) {
        status_->setText("Lỗi dữ liệu: " + session_.error());
        setContent(label("Không thể nạp dữ liệu phim và rạp. " + session_.error(), "sectionTitle"), "home");
    } else {
        status_->setText(session_.notice().isEmpty() ?
                         "Dữ liệu đã sẵn sàng. Thanh toán chỉ là mô phỏng." : session_.notice());
        showHome();
    }
}

void V2Window::setContent(QWidget* page, const QString& section) {
    while (QLayoutItem* item = contentLayout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    contentLayout_->addWidget(page);
    contentLayout_->addStretch();
    homeNav_->setProperty("role", section == "home" ? "navActive" : "nav");
    ticketsNav_->setProperty("role", section == "orders" ? "navActive" : "nav");
    homeNav_->style()->unpolish(homeNav_); homeNav_->style()->polish(homeNav_);
    ticketsNav_->style()->unpolish(ticketsNav_); ticketsNav_->style()->polish(ticketsNav_);
    if (auto* scroll = qobject_cast<QScrollArea*>(content_->parentWidget()->parentWidget()))
        scroll->verticalScrollBar()->setValue(0);
}

void V2Window::showHome() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(24);
    auto* hero = new MovieBanner(session_.featuredMovies(), assets_);
    hero->setOnDetails([this](int id) { branchId_ = 0; showMovie(id); });
    hero->setOnBook([this](int id) {
        branchId_ = 0;
        showMovie(id);
        if (auto* branch = content_->findChild<QComboBox*>("branchSelect"))
            if (auto* scroll = qobject_cast<QScrollArea*>(content_->parentWidget()->parentWidget()))
                scroll->ensureWidgetVisible(branch);
    });
    root->addWidget(hero);

    auto* quick = panel();
    auto* quickLayout = new QHBoxLayout(quick);
    quickLayout->setContentsMargins(22, 17, 22, 17);
    quickLayout->setSpacing(12);
    quickLayout->addWidget(label("ĐẶT VÉ NHANH", "eyebrow"));
    auto* movieBox = new QComboBox;
    movieBox->addItem("Chọn phim", 0);
    for (const auto* movie : session_.featuredMovies())
        movieBox->addItem(QString::fromStdString(movie->title), movie->movieId);
    auto* branchBox = new QComboBox;
    branchBox->addItem("Chọn rạp", 0);
    for (const auto& branch : session_.cinema().branches().all())
        branchBox->addItem(QString::fromStdString(branch.name), branch.id);
    quickLayout->addWidget(movieBox, 2);
    quickLayout->addWidget(branchBox, 2);
    auto* quickButton = action("Xem suất chiếu  →", true);
    quickLayout->addWidget(quickButton);
    connect(quickButton, &QPushButton::clicked, this, [this, movieBox, branchBox] {
        const int movie = movieBox->currentData().toInt();
        if (movie == 0) { status_->setText("Vui lòng chọn một phim."); return; }
        branchId_ = branchBox->currentData().toInt();
        showMovie(movie);
    });
    root->addWidget(quick);
    auto* gallery = new MovieGallery(session_, assets_);
    gallery->setOnMovie([this](int id) { branchId_ = 0; showMovie(id); });
    root->addWidget(gallery);
    setContent(page, "home");
}

void V2Window::showMovie(int movieId) {
    movieId_ = movieId;
    const MovieDetails* movie = session_.movies().find(movieId);
    if (!movie || !session_.isFeaturedMovie(movieId)) { showHome(); return; }
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setSpacing(19);
    auto* back = action("← Danh sách phim");
    connect(back, &QPushButton::clicked, this, [this] { showHome(); });
    root->addWidget(back, 0, Qt::AlignLeft);
    auto* detailBanner = new MovieBanner({movie}, assets_);
    root->addWidget(detailBanner);
    auto* info = panel();
    auto* row = new QHBoxLayout(info);
    row->setContentsMargins(22, 22, 22, 22);
    row->setSpacing(25);
    auto* poster = new QLabel;
    poster->setFixedSize(205, 290);
    QPixmap art(posterFile(assets_, *movie));
    if (!art.isNull()) poster->setPixmap(art.scaled(poster->size(), Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));
    row->addWidget(poster);
    auto* details = new QWidget;
    auto* detailLayout = new QVBoxLayout(details);
    detailLayout->setSpacing(9);
    detailLayout->addWidget(label("CHI TIẾT PHIM", "eyebrow"));
    detailLayout->addWidget(label(titleOf(movie), "sectionTitle"));
    if (!movie->originalTitle.empty())
        detailLayout->addWidget(label(QString::fromStdString(movie->originalTitle), "muted"));
    detailLayout->addWidget(label(QString("%1  •  %2 phút  •  %3")
        .arg(QString::fromStdString(movie->genre)).arg(movie->durationMinutes)
        .arg(movie->ageRating.empty() ? "Chưa phân loại" : QString::fromStdString(movie->ageRating))));
    detailLayout->addWidget(label("Khởi chiếu: " + QString::fromStdString(movie->releaseDate)));
    if (!movie->director.empty()) detailLayout->addWidget(label("Đạo diễn: " + QString::fromStdString(movie->director)));
    if (!movie->cast.empty()) detailLayout->addWidget(label("Diễn viên: " + QString::fromStdString(movie->cast)));
    detailLayout->addWidget(label(QString::fromStdString(movie->synopsis), "muted"));
    detailLayout->addStretch();
    row->addWidget(details, 1);
    root->addWidget(info);
    root->addWidget(label("CHỌN CỤM RẠP & SUẤT CHIẾU", "eyebrow"));
    root->addWidget(label("Lịch chiếu dưới đây là dữ liệu mô phỏng của OOP Cinema.", "muted"));
    auto* branchBox = new QComboBox;
    branchBox->setObjectName("branchSelect");
    for (const auto& branch : session_.cinema().branches().all())
        branchBox->addItem(QString::fromStdString(branch.name), branch.id);
    for (int i = 0; i < branchBox->count(); ++i)
        if (branchBox->itemData(i).toInt() == branchId_) branchBox->setCurrentIndex(i);
    root->addWidget(branchBox);
    auto* showsHost = new QWidget;
    auto* showsLayout = new QVBoxLayout(showsHost);
    root->addWidget(showsHost);
    auto scrollTo = [this](QWidget* target) {
        if (auto* scroll = qobject_cast<QScrollArea*>(content_->parentWidget()->parentWidget()))
            scroll->ensureWidgetVisible(target);
    };
    detailBanner->setOnBook([scrollTo, branchBox](int) { scrollTo(branchBox); });
    detailBanner->setOnDetails([scrollTo, info](int) { scrollTo(info); });
    auto populate = [this, movieId, branchBox, showsLayout] {
        while (QLayoutItem* item = showsLayout->takeAt(0)) {
            delete item->widget(); delete item;
        }
        branchId_ = branchBox->currentData().toInt();
        int count = 0;
        for (const Showtime& show : session_.cinema().schedule().getShowtimes()) {
            const CinemaBranch* branch = session_.cinema().branches().findByRoom(show.getRoomId());
            if (show.getMovieId() != movieId || !branch || branch->id != branchId_ ||
                !session_.cinema().isBookable(show.getId())) continue;
            const CinemaRoom* room = nullptr;
            for (const auto& item : session_.cinema().rooms())
                if (item.getId() == show.getRoomId()) room = &item;
            auto* button = action(longDate(show.getStartTime()) + "  •  " +
                (room ? QString::fromStdString(room->getName()) : QString("Phòng")) +
                "  •  từ " + vnd(show.getPriceVnd()) + "  →");
            const int id = show.getId();
            button->setObjectName(QString("showtime-%1").arg(id));
            connect(button, &QPushButton::clicked, this, [this, id] { showSeats(id); });
            showsLayout->addWidget(button);
            ++count;
        }
        if (count == 0) showsLayout->addWidget(label("Chưa có suất chiếu phù hợp ở cụm rạp này.", "muted"));
    };
    connect(branchBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [populate](int) { populate(); });
    populate();
    setContent(page, "movie");
}

void V2Window::showSeats(int showtimeId) {
    if (showtimeId_ != showtimeId) seats_.clear();
    showtimeId_ = showtimeId;
    const Showtime* show = session_.cinema().schedule().findShowtime(showtimeId);
    if (!show) { showMovie(movieId_); return; }
    movieId_ = show->getMovieId();
    const MovieDetails* movie = session_.movies().find(movieId_);
    const CinemaBranch* branch = session_.cinema().branches().findByRoom(show->getRoomId());
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setSpacing(18);
    auto* back = action("← Đổi suất chiếu");
    connect(back, &QPushButton::clicked, this, [this] { showMovie(movieId_); });
    root->addWidget(back, 0, Qt::AlignLeft);
    root->addWidget(label("CHỌN GHẾ", "eyebrow"));
    root->addWidget(label(titleOf(movie), "sectionTitle"));
    root->addWidget(label((branch ? QString::fromStdString(branch->name) : "") +
                           "  •  " + longDate(show->getStartTime()), "muted"));
    auto* seatPanel = panel();
    auto* seatPanelLayout = new QVBoxLayout(seatPanel);
    seatPanelLayout->setContentsMargins(20, 24, 20, 24);
    auto* map = new SeatMapWidget(session_, showtimeId);
    map->setSelected(seats_);
    seatPanelLayout->addWidget(map);
    root->addWidget(seatPanel);
    auto* summary = label("", "price");
    root->addWidget(summary);
    auto* next = action("Tiếp tục chọn bắp nước  →", true);
    next->setObjectName("openSnacks");
    root->addWidget(next, 0, Qt::AlignRight);
    auto update = [this, map, summary, next, showtimeId] {
        seats_ = map->selected();
        long long total = 0;
        for (const auto& seat : seats_) total += session_.ticketPrice(showtimeId, seat);
        summary->setText(QString("%1 ghế: %2   •   %3").arg(seats_.size())
                         .arg(joinSeats(seats_)).arg(vnd(total)));
        next->setEnabled(!seats_.empty());
    };
    map->setOnChange(update);
    update();
    connect(next, &QPushButton::clicked, this, [this] { showSnacks(); });
    setContent(page, "seats");
}

void V2Window::showSnacks() {
    if (quantities_.size() != session_.snacks().size())
        quantities_.assign(session_.snacks().size(), 0);
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setSpacing(17);
    auto* back = action("← Đổi ghế");
    back->setObjectName("backToSeats");
    connect(back, &QPushButton::clicked, this, [this] { showSeats(showtimeId_); });
    root->addWidget(back, 0, Qt::AlignLeft);
    root->addWidget(label("BẮP NƯỚC", "eyebrow"));
    root->addWidget(label("Thêm chút vị ngon cho buổi chiếu", "sectionTitle"));
    root->addWidget(label("Bạn có thể bỏ qua. Giá là dữ liệu mô phỏng của OOP Cinema.", "muted"));
    auto* total = label("", "price");
    total->setObjectName("snack-total");
    auto updateTotal = [this, total] {
        long long tickets = 0;
        for (const auto& seat : seats_)
            tickets += session_.ticketPrice(showtimeId_, seat);
        const long long food = session_.snackTotal(quantities_);
        const long long discount = session_.discountFor(voucher_, tickets + food);
        total->setText("Vé " + vnd(tickets) + "  +  Bắp nước " + vnd(food) +
                       "  −  Ưu đãi " + vnd(discount) +
                       "\nTổng đơn: " + vnd(tickets + food - discount));
    };
    for (size_t i = 0; i < session_.snacks().size(); ++i) {
        const SnackItem& item = session_.snacks()[i];
        auto* card = panel();
        card->setObjectName("snackCard");
        card->setProperty("snackId", item.id);
        auto* row = new QHBoxLayout(card);
        row->setContentsMargins(12, 8, 14, 8);
        row->setSpacing(14);
        auto* photo = new QLabel;
        photo->setObjectName(QString("snack-image-%1").arg(item.id));
        photo->setFixedSize(116, 116);
        photo->setAlignment(Qt::AlignCenter);
        const QPixmap image(QDir(assets_).filePath(item.imageFile));
        if (image.isNull()) photo->setText("Ảnh không tải được");
        else photo->setPixmap(image.scaled(photo->size(), Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
        row->addWidget(photo);
        auto* info = new QWidget;
        auto* words = new QVBoxLayout(info);
        words->setContentsMargins(0, 0, 0, 0);
        words->setSpacing(5);
        auto* name = label(item.name, "snackName");
        name->setObjectName(QString("snack-name-%1").arg(item.id));
        words->addWidget(name);
        auto* contents = label(item.contents, "snackContents");
        contents->setObjectName(QString("snack-contents-%1").arg(item.id));
        words->addWidget(contents);
        auto* price = label(vnd(item.price), "snackPrice");
        price->setObjectName(QString("snack-price-%1").arg(item.id));
        words->addWidget(price);
        row->addWidget(info, 1);
        auto* controls = new QHBoxLayout;
        controls->setSpacing(5);
        auto* minus = action("−");
        auto* plus = action("+");
        auto* count = label(QString::number(quantities_[i]), "snackCount");
        minus->setObjectName(QString("snack-minus-%1").arg(item.id));
        plus->setObjectName(QString("snack-plus-%1").arg(item.id));
        count->setObjectName(QString("snack-count-%1").arg(item.id));
        minus->setProperty("role", "quantity");
        plus->setProperty("role", "quantity");
        minus->setFixedSize(40, 40);
        plus->setFixedSize(40, 40);
        count->setFixedWidth(28);
        count->setAlignment(Qt::AlignCenter);
        minus->setEnabled(quantities_[i] > 0);
        plus->setEnabled(quantities_[i] < 9);
        controls->addWidget(minus);
        controls->addWidget(count);
        controls->addWidget(plus);
        row->addLayout(controls);
        auto change = [this, i, minus, plus, count, updateTotal](int step) {
            quantities_[i] = std::max(0, std::min(9, quantities_[i] + step));
            count->setText(QString::number(quantities_[i]));
            minus->setEnabled(quantities_[i] > 0);
            plus->setEnabled(quantities_[i] < 9);
            updateTotal();
        };
        connect(minus, &QPushButton::clicked, this, [change] { change(-1); });
        connect(plus, &QPushButton::clicked, this, [change] { change(1); });
        root->addWidget(card);
    }
    updateTotal();
    root->addWidget(total);
    auto* next = action("Tiếp tục thanh toán  →", true);
    next->setObjectName("openCheckout");
    connect(next, &QPushButton::clicked, this, [this] { showCheckout(); });
    root->addWidget(next, 0, Qt::AlignRight);
    setContent(page, "snacks");
}

void V2Window::showCheckout() {
    const Showtime* show = session_.cinema().schedule().findShowtime(showtimeId_);
    if (!show || seats_.empty()) { showHome(); return; }
    const MovieDetails* movie = session_.movies().find(show->getMovieId());
    const CinemaBranch* branch = session_.cinema().branches().findByRoom(show->getRoomId());
    const QString confirmation = "Phim: " + titleOf(movie) + "\nRạp: " +
        (branch ? QString::fromStdString(branch->name) : QString()) +
        "\nSuất: " + longDate(show->getStartTime()) +
        "\nGhế: " + joinSeats(seats_);
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setSpacing(17);
    auto* back = action("← Bắp nước");
    back->setObjectName("backToSnacks");
    connect(back, &QPushButton::clicked, this, [this] { showSnacks(); });
    root->addWidget(back, 0, Qt::AlignLeft);
    root->addWidget(label("KIỂM TRA & THANH TOÁN", "eyebrow"));
    root->addWidget(label("Thông tin đơn hàng", "sectionTitle"));
    auto* orderPanel = panel();
    auto* orderLayout = new QVBoxLayout(orderPanel);
    orderLayout->setContentsMargins(22, 20, 22, 20);
    orderLayout->addWidget(label(titleOf(movie), "orderMovie"));
    orderLayout->addWidget(label((branch ? QString::fromStdString(branch->name) : "") +
                                  "  •  " + longDate(show->getStartTime()), "orderFact"));
    orderLayout->addWidget(label("Ghế " + joinSeats(seats_), "orderFact"));
    root->addWidget(orderPanel);
    root->addWidget(label("THÔNG TIN NGƯỜI MUA — KHÔNG CẦN TÀI KHOẢN", "eyebrow"));
    auto* name = new QLineEdit; name->setPlaceholderText("Họ và tên");
    auto* phone = new QLineEdit; phone->setPlaceholderText("Số điện thoại");
    auto* email = new QLineEdit; email->setPlaceholderText("Email nhận mã vé");
    root->addWidget(name); root->addWidget(phone); root->addWidget(email);
    auto* voucher = new QComboBox;
    voucher->addItem("Không dùng ưu đãi", "");
    for (const auto& item : session_.vouchers())
        voucher->addItem(item.code + "  —  " + item.description, item.code);
    for (int i = 0; i < voucher->count(); ++i)
        if (voucher->itemData(i).toString() == voucher_) voucher->setCurrentIndex(i);
    root->addWidget(voucher);
    auto* total = label("", "price");
    auto update = [this, showtimeId=showtimeId_, voucher, total] {
        voucher_ = voucher->currentData().toString();
        long long seatsTotal = 0;
        for (const auto& seat : seats_) seatsTotal += session_.ticketPrice(showtimeId, seat);
        const long long food = session_.snackTotal(quantities_);
        const long long discount = session_.discountFor(voucher_, seatsTotal + food);
        total->setText("Vé " + vnd(seatsTotal) + "  +  Bắp nước " + vnd(food) +
                       "  −  Ưu đãi " + vnd(discount) + "\nTHÀNH TIỀN  " +
                       vnd(seatsTotal + food - discount));
    };
    connect(voucher, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [update](int) { update(); });
    update();
    root->addWidget(total);
    auto* payNow = new QRadioButton("Thanh toán mô phỏng ngay — vé Paid");
    auto* pending = new QRadioButton("Giữ vé Pending — thanh toán demo sau");
    payNow->setChecked(true);
    root->addWidget(payNow); root->addWidget(pending);
    root->addWidget(label("Không có giao dịch ngân hàng thật. Chuyển khoản không tự xác nhận.", "muted"));
    auto* confirm = action("Xác nhận đặt vé  →", true);
    confirm->setObjectName("confirmCheckout");
    connect(confirm, &QPushButton::clicked, this,
            [this, name, phone, email, payNow, total, confirmation] {
        if (QMessageBox::question(this, "Xác nhận đặt vé",
            confirmation + "\nNgười mua: " + name->text().trimmed() +
            "\n\n" + total->text() +
            "\n\nTiếp tục tạo đơn?") != QMessageBox::Yes) return;
        QString code;
        const std::vector<std::string> seats(seats_.begin(), seats_.end());
        if (!session_.checkout(showtimeId_, seats, quantities_, voucher_,
                               name->text(), phone->text(), email->text(),
                               payNow->isChecked(), code)) {
            QMessageBox::warning(this, "Không thể đặt vé", session_.error());
            showSeats(showtimeId_);
            return;
        }
        const bool saved = saveChanges();
        status_->setText(saved ? "Đã tạo và lưu đơn " + code + "." :
                          "Đơn " + code + " chỉ đang ở bộ nhớ; chưa lưu được. Vui lòng kiểm tra lỗi.");
        showTicket(code);
    });
    root->addWidget(confirm, 0, Qt::AlignRight);
    setContent(page, "checkout");
}

void V2Window::showTicket(const QString& code) {
    const CinemaOrder* order = nullptr;
    for (const auto& item : session_.orders()) if (item.code == code) order = &item;
    if (!order) { showOrders(); return; }
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setSpacing(17);
    const QString state = statusOf(session_, *order);
    root->addWidget(label(state == "Đã hủy" ? "ĐƠN ĐÃ HỦY" :
                          state == "Đang chờ thanh toán" ? "VÉ ĐANG GIỮ" :
                          "VÉ ĐIỆN TỬ", "eyebrow"));
    root->addWidget(label(state, "sectionTitle"));
    root->addWidget(new TicketCard(session_, *order));
    auto* buttons = new QHBoxLayout;
    auto* pay = action("Thanh toán demo", true);
    auto* cancel = action("Hủy vé");
    pay->setObjectName("payOrder");
    cancel->setObjectName("cancelOrder");
    const bool isPending = state == "Đang chờ thanh toán";
    const bool isCancelled = state == "Đã hủy";
    pay->setEnabled(isPending);
    cancel->setEnabled(!isCancelled);
    buttons->addWidget(pay); buttons->addWidget(cancel);
    buttons->addStretch();
    root->addLayout(buttons);
    connect(pay, &QPushButton::clicked, this, [this, code] {
        if (!session_.pay(code)) QMessageBox::warning(this, "Thanh toán", session_.error());
        else {
            const bool saved = saveChanges();
            status_->setText(saved ? "Thanh toán mô phỏng đã hoàn tất và lưu." :
                              "Vé đã chuyển Paid trong bộ nhớ nhưng chưa lưu được.");
        }
        showTicket(code);
    });
    connect(cancel, &QPushButton::clicked, this, [this, code] {
        if (QMessageBox::question(this, "Hủy vé", "Hủy toàn bộ vé trong đơn " + code + "?")
            != QMessageBox::Yes) return;
        if (!session_.cancel(code)) QMessageBox::warning(this, "Hủy vé", session_.error());
        else {
            const bool saved = saveChanges();
            status_->setText(saved ? "Đã hủy và lưu đơn. Ghế đã trở lại trạng thái trống." :
                              "Đơn đã hủy trong bộ nhớ nhưng chưa lưu được.");
        }
        showTicket(code);
    });
    auto* more = action("Xem tất cả vé  →");
    connect(more, &QPushButton::clicked, this, [this] { showOrders(); });
    root->addWidget(more, 0, Qt::AlignLeft);
    setContent(page, "ticket");
}

void V2Window::showOrders() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setSpacing(15);
    root->addWidget(label("VÉ CỦA TÔI", "eyebrow"));
    root->addWidget(label("Các đơn đã đặt", "sectionTitle"));
    if (session_.orders().empty()) {
        root->addWidget(label("Chưa có vé. Hãy chọn một phim để bắt đầu.", "muted"));
    }
    for (const auto& order : session_.orders()) {
        auto* row = panel();
        auto* layout = new QHBoxLayout(row);
        layout->addWidget(label(order.code + "   •   " +
            titleOf(session_.movies().find(order.movieId)) + "   •   " +
            statusOf(session_, order)), 1);
        auto* view = action("Xem vé  →");
        connect(view, &QPushButton::clicked, this,
                [this, code=order.code] { showTicket(code); });
        layout->addWidget(view);
        root->addWidget(row);
    }
    setContent(page, "orders");
}

bool V2Window::saveChanges() {
    bool replaceDamaged = false;
    if (session_.hasDamagedSave()) {
        if (QMessageBox::warning(this, "Bản lưu cũ bị lỗi",
            "Bản lưu cũ đang bị lỗi hoặc thiếu file. Ứng dụng sẽ sao lưu các file cũ vào "
            "thư mục damaged-backup trước khi ghi dữ liệu demo. Tiếp tục?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
            return false;
        replaceDamaged = true;
    }
    if (!session_.save(replaceDamaged)) {
        QMessageBox::warning(this, "Lưu thất bại", session_.error());
        return false;
    }
    return true;
}

void V2Window::closeEvent(QCloseEvent* event) {
    if (!session_.dirty()) { event->accept(); return; }
    if (saveChanges()) event->accept();
    else event->ignore();
}
