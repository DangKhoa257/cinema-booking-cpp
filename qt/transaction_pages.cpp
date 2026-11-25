#include "ui_pages.h"
#include "ui_helpers.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <utility>

SeatMapPage::SeatMapPage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("seatMapPage");
    auto* root = pageLayout(this, "Chọn ghế", "Ghế được giữ riêng theo từng suất chiếu. Chọn suất, khách và vị trí.");
    auto* controls = card(this);
    auto* controlsLayout = new QHBoxLayout(controls);
    controlsLayout->setContentsMargins(20, 18, 20, 18);
    showtimeBox_ = new QComboBox(controls);
    showtimeBox_->setObjectName("showtimeSelect");
    customerBox_ = new QComboBox(controls);
    customerBox_->setObjectName("customerSelect");
    controlsLayout->addWidget(mutedLabel("SUẤT CHIẾU", controls));
    controlsLayout->addWidget(showtimeBox_, 2);
    controlsLayout->addSpacing(14);
    controlsLayout->addWidget(mutedLabel("KHÁCH", controls));
    controlsLayout->addWidget(customerBox_, 1);
    root->addWidget(controls);
    auto* map = card(this);
    auto* mapLayout = new QVBoxLayout(map);
    mapLayout->setContentsMargins(26, 24, 26, 24);
    auto* screen = new QLabel("MÀN HÌNH", map);
    screen->setObjectName("screen");
    screen->setAlignment(Qt::AlignCenter);
    mapLayout->addWidget(screen);
    seatGrid_ = new QGridLayout;
    seatGrid_->setAlignment(Qt::AlignCenter);
    seatGrid_->setHorizontalSpacing(12);
    seatGrid_->setVerticalSpacing(12);
    mapLayout->addLayout(seatGrid_);
    mapLayout->addWidget(mutedLabel("● Trống     ● Đang giữ     ● Đã đặt", map));
    root->addWidget(map);
    auto* checkout = card(this);
    auto* checkoutLayout = new QHBoxLayout(checkout);
    checkoutLayout->setContentsMargins(20, 16, 20, 16);
    checkoutLayout->addWidget(mutedLabel("THANH TOÁN", checkout));
    methodBox_ = new QComboBox(checkout);
    methodBox_->setObjectName("bookingMethodSelect");
    methodBox_->addItems({"Thanh toán demo", "Chuyển khoản demo (giữ Pending)"});
    checkoutLayout->addWidget(methodBox_, 1);
    auto* bookButton = new QPushButton("Đặt ghế", checkout);
    bookButton->setObjectName("bookButton");
    checkoutLayout->addWidget(bookButton);
    root->addWidget(checkout);
    root->addStretch();
    connect(showtimeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this] { selectedSeat_.clear(); drawSeats(); });
    connect(bookButton, &QPushButton::clicked, this, [this] {
        if (selectedSeat_.isEmpty() || showtimeBox_->currentIndex() < 0 ||
            customerBox_->currentIndex() < 0) {
            callbacks_.notify("Chọn suất chiếu, khách hàng và ghế trống.");
            return;
        }
        int ticketId = session_.book(customerBox_->currentData().toInt(),
            showtimeBox_->currentData().toInt(), selectedSeat_.toStdString(),
            methodBox_->currentIndex() == 0);
        if (!ticketId) { callbacks_.notify("Không thể đặt ghế này."); return; }
        callbacks_.notify(QString("Đã tạo vé #%1. Xem trạng thái ở trang Vé.").arg(ticketId));
        selectedSeat_.clear();
        callbacks_.changed();
    });
    refresh();
}

void SeatMapPage::refresh() {
    const int oldShow = showtimeBox_->currentData().toInt();
    const int oldCustomer = customerBox_->currentData().toInt();
    showtimeBox_->blockSignals(true);
    showtimeBox_->clear();
    for (const Showtime& show : session_.schedule().getShowtimes()) {
        const Movie* movie = session_.movies().findMovie(show.getMovieId());
        showtimeBox_->addItem(QString("#%1  %2  •  %3")
            .arg(show.getId())
            .arg(movie ? QString::fromStdString(movie->getTitle()) : "Không có phim")
            .arg(QString::fromStdString(show.getStartTime())), show.getId());
    }
    int showIndex = showtimeBox_->findData(oldShow);
    if (showIndex >= 0) showtimeBox_->setCurrentIndex(showIndex);
    showtimeBox_->blockSignals(false);
    customerBox_->clear();
    for (const Customer& customer : session_.users().getCustomers())
        customerBox_->addItem(QString("#%1  %2").arg(customer.getId())
            .arg(QString::fromStdString(customer.getName())), customer.getId());
    int customerIndex = customerBox_->findData(oldCustomer);
    if (customerIndex >= 0) customerBox_->setCurrentIndex(customerIndex);
    drawSeats();
}

void SeatMapPage::drawSeats() {
    clearLayout(seatGrid_);
    const Showtime* show = session_.schedule().findShowtime(showtimeBox_->currentData().toInt());
    if (!show) return;
    const CinemaRoom* room = nullptr;
    for (const CinemaRoom& candidate : session_.rooms())
        if (candidate.getId() == show->getRoomId()) room = &candidate;
    if (!room) return;
    for (const Seat& seat : room->getSeats()) {
        const QString id = QString::fromStdString(seat.getSeatId());
        auto* button = new QPushButton(id, this);
        button->setObjectName("seat");
        button->setProperty("seatState", session_.seatStatus(show->getId(), seat.getSeatId()) ==
            SeatStatus::Available ? "available" : "occupied");
        const bool available = session_.seatStatus(show->getId(), seat.getSeatId()) ==
                               SeatStatus::Available;
        button->setEnabled(available);
        button->setCheckable(true);
        button->setChecked(id == selectedSeat_);
        button->setFixedSize(72, 54);
        connect(button, &QPushButton::clicked, this, [this, id] {
            selectedSeat_ = id;
            drawSeats();
        });
        seatGrid_->addWidget(button, seat.getRow() - 'A', seat.getNumber() - 1);
    }
}

CustomerPage::CustomerPage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("customerPage");
    auto* root = pageLayout(this, "Khách hàng", "Đăng ký khách demo trước khi đặt vé.");
    auto* form = card(this);
    auto* row = new QHBoxLayout(form);
    row->setContentsMargins(20, 18, 20, 18);
    idEdit_ = new QLineEdit(form);
    idEdit_->setObjectName("customerIdInput");
    idEdit_->setPlaceholderText("ID mới");
    nameEdit_ = new QLineEdit(form);
    nameEdit_->setObjectName("customerNameInput");
    nameEdit_->setPlaceholderText("Tên khách hàng");
    row->addWidget(idEdit_, 1);
    row->addWidget(nameEdit_, 3);
    auto* add = new QPushButton("Thêm khách", form);
    add->setObjectName("primary");
    row->addWidget(add);
    root->addWidget(form);
    root->addWidget(mutedLabel("DANH SÁCH KHÁCH", this));
    auto* list = new QWidget(this);
    customerList_ = new QVBoxLayout(list);
    customerList_->setContentsMargins(0, 0, 0, 0);
    root->addWidget(list);
    root->addStretch();
    connect(add, &QPushButton::clicked, this, [this] {
        bool parsed = false;
        int id = idEdit_->text().toInt(&parsed);
        if (!parsed || !session_.addCustomer(id, nameEdit_->text().trimmed().toUtf8().toStdString())) {
            callbacks_.notify("ID hoặc tên không hợp lệ, hoặc ID đã tồn tại.");
            return;
        }
        idEdit_->clear(); nameEdit_->clear();
        callbacks_.notify("Đã thêm khách hàng.");
        callbacks_.changed();
    });
    refresh();
}

void CustomerPage::refresh() {
    clearLayout(customerList_);
    for (const Customer& customer : session_.users().getCustomers()) {
        auto* frame = card(this);
        auto* row = new QHBoxLayout(frame);
        row->addWidget(new QLabel(QString("#%1").arg(customer.getId()), frame));
        auto* name = new QLabel(QString::fromStdString(customer.getName()), frame);
        name->setObjectName("cardTitle");
        row->addWidget(name, 1);
        row->addWidget(mutedLabel("KHÁCH DEMO", frame));
        customerList_->addWidget(frame);
    }
}

TicketPage::TicketPage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("ticketPage");
    auto* root = pageLayout(this, "Vé của bạn", "Theo dõi trạng thái và hủy vé khi cần.");
    auto* list = new QWidget(this);
    ticketList_ = new QVBoxLayout(list);
    ticketList_->setContentsMargins(0, 0, 0, 0);
    ticketList_->setSpacing(12);
    root->addWidget(list);
    root->addStretch();
    refresh();
}

void TicketPage::refresh() {
    clearLayout(ticketList_);
    if (session_.tickets().empty()) ticketList_->addWidget(mutedLabel("Chưa có vé. Hãy chọn một suất chiếu.", this));
    for (const Ticket& ticket : session_.tickets()) {
        auto* frame = card(this);
        auto* row = new QHBoxLayout(frame);
        row->setContentsMargins(20, 18, 20, 18);
        auto* details = new QVBoxLayout;
        auto* title = new QLabel(QString("Vé #%1  •  Ghế %2")
            .arg(ticket.getId()).arg(QString::fromStdString(ticket.getSeatId())), frame);
        title->setObjectName("cardTitle");
        details->addWidget(title);
        details->addWidget(mutedLabel(QString("Khách #%1  •  Suất #%2  •  %3 đ")
            .arg(ticket.getCustomerId()).arg(ticket.getShowtimeId())
            .arg(ticket.getPriceVnd()), frame));
        row->addLayout(details, 1);
        const char* status = ticket.getStatus() == TicketStatus::Paid ? "Paid" :
                             ticket.getStatus() == TicketStatus::Pending ? "Pending" : "Cancelled";
        auto* badge = new QLabel(status, frame);
        badge->setObjectName("badge");
        row->addWidget(badge);
        if (ticket.getStatus() != TicketStatus::Cancelled) {
            auto* cancel = new QPushButton("Hủy vé", frame);
            int id = ticket.getId();
            connect(cancel, &QPushButton::clicked, this, [this, id] {
                callbacks_.notify(session_.cancel(id) ? "Đã hủy vé; ghế trở lại trống." :
                                                     "Không thể hủy vé.");
                callbacks_.changed();
            });
            row->addWidget(cancel);
        }
        ticketList_->addWidget(frame);
    }
}

PaymentPage::PaymentPage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("paymentPage");
    auto* root = pageLayout(this, "Thanh toán mô phỏng", "Chỉ vé Pending có thể được thanh toán lại.");
    auto* note = card(this);
    auto* noteLayout = new QVBoxLayout(note);
    noteLayout->setContentsMargins(22, 20, 22, 20);
    auto* heading = new QLabel("Không có giao dịch ngân hàng thật", note);
    heading->setObjectName("cardTitle");
    noteLayout->addWidget(heading);
    noteLayout->addWidget(mutedLabel("Chuyển khoản demo không tự xác nhận tiền. Vé tiếp tục Pending cho đến khi chọn thanh toán demo hợp lệ hoặc hủy vé.", note));
    root->addWidget(note);
    auto* form = card(this);
    auto* row = new QHBoxLayout(form);
    row->setContentsMargins(22, 20, 22, 20);
    ticketBox_ = new QComboBox(form);
    ticketBox_->setObjectName("pendingTicketSelect");
    methodBox_ = new QComboBox(form);
    methodBox_->addItems({"Thanh toán demo", "Chuyển khoản demo"});
    row->addWidget(ticketBox_, 2);
    row->addWidget(methodBox_, 2);
    auto* pay = new QPushButton("Xử lý", form);
    pay->setObjectName("primary");
    row->addWidget(pay);
    root->addWidget(form);
    root->addStretch();
    connect(pay, &QPushButton::clicked, this, [this] {
        if (ticketBox_->currentIndex() < 0) {
            callbacks_.notify("Không có vé Pending để thanh toán."); return;
        }
        bool paid = session_.pay(ticketBox_->currentData().toInt(), methodBox_->currentIndex() == 0);
        callbacks_.notify(paid ? "Vé đã Paid (thanh toán demo)." :
                                 "Chưa xác nhận thanh toán; vé vẫn Pending.");
        callbacks_.changed();
    });
    refresh();
}

void PaymentPage::refresh() {
    ticketBox_->clear();
    for (const Ticket& ticket : session_.tickets()) {
        if (ticket.getStatus() == TicketStatus::Pending)
            ticketBox_->addItem(QString("Vé #%1  •  Ghế %2  •  %3 đ")
                .arg(ticket.getId()).arg(QString::fromStdString(ticket.getSeatId()))
                .arg(ticket.getPriceVnd()), ticket.getId());
    }
}
