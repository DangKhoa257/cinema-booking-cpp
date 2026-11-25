#include "ui_pages.h"
#include "ui_helpers.h"
#include "ui_dialogs.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <utility>

SeatMapPage::SeatMapPage(UiSession& session, UiCallbacks callbacks, QWidget* parent)
    : QWidget(parent), session_(session), callbacks_(std::move(callbacks)) {
    setObjectName("seatMapPage");
    auto* root = pageLayout(this, "Chọn ghế", "Ghế được giữ riêng theo từng suất chiếu. Chọn suất, khách và vị trí.");
    auto* controls = card(this);
    auto* controlsLayout = new QGridLayout(controls);
    controlsLayout->setContentsMargins(20, 18, 20, 18);
    showtimeBox_ = new QComboBox(controls);
    showtimeBox_->setObjectName("showtimeSelect");
    customerBox_ = new QComboBox(controls);
    customerBox_->setObjectName("customerSelect");
    showtimeBox_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    customerBox_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    controlsLayout->addWidget(mutedLabel("SUẤT CHIẾU", controls), 0, 0);
    controlsLayout->addWidget(mutedLabel("KHÁCH", controls), 0, 1);
    controlsLayout->addWidget(showtimeBox_, 1, 0);
    controlsLayout->addWidget(customerBox_, 1, 1);
    controlsLayout->setColumnStretch(0, 2);
    controlsLayout->setColumnStretch(1, 1);
    root->addWidget(controls);
    auto* map = card(this);
    map->setMinimumHeight(320);
    auto* mapLayout = new QVBoxLayout(map);
    mapLayout->setContentsMargins(26, 24, 26, 24);
    mapLayout->setSpacing(14);
    auto* screen = new QLabel("MÀN HÌNH", map);
    screen->setObjectName("screen");
    screen->setAlignment(Qt::AlignCenter);
    mapLayout->addWidget(screen);
    roomLabel_ = mutedLabel("", map);
    roomLabel_->setObjectName("roomName");
    mapLayout->addWidget(roomLabel_);
    seatGrid_ = new QGridLayout;
    seatGrid_->setAlignment(Qt::AlignCenter);
    seatGrid_->setHorizontalSpacing(12);
    seatGrid_->setVerticalSpacing(12);
    mapLayout->addLayout(seatGrid_);
    auto* legend = new QHBoxLayout;
    for (const auto& item : {std::pair<const char*, const char*>{"● Trống", "available"},
                             {"● Đang giữ", "held"}, {"● Đã đặt", "booked"},
                             {"● Đang chọn", "selected"}}) {
        auto* label = new QLabel(item.first, map);
        label->setObjectName("legend");
        label->setProperty("legendState", item.second);
        legend->addWidget(label);
    }
    legend->addStretch();
    mapLayout->addLayout(legend);
    root->addWidget(map);
    confirmationLabel_ = mutedLabel("Chọn suất, khách hàng và ghế để xem thông tin vé.", this);
    confirmationLabel_->setObjectName("bookingConfirmation");
    root->addWidget(confirmationLabel_);
    auto* checkout = card(this);
    auto* checkoutLayout = new QVBoxLayout(checkout);
    checkoutLayout->setContentsMargins(20, 16, 20, 16);
    checkoutLayout->addWidget(mutedLabel("THANH TOÁN", checkout));
    auto* checkoutRow = new QHBoxLayout;
    methodBox_ = new QComboBox(checkout);
    methodBox_->setObjectName("bookingMethodSelect");
    methodBox_->addItems({"Thanh toán demo", "Chuyển khoản demo (giữ Pending)"});
    checkoutRow->addWidget(methodBox_, 1);
    bookButton_ = new QPushButton("Xác nhận đặt vé", checkout);
    bookButton_->setObjectName("bookButton");
    checkoutRow->addWidget(bookButton_);
    checkoutLayout->addLayout(checkoutRow);
    root->addWidget(checkout);
    root->addStretch();
    connect(showtimeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this] { selectedSeat_.clear(); drawSeats(); });
    connect(customerBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this] { updateConfirmation(); });
    connect(bookButton_, &QPushButton::clicked, this, [this] {
        if (selectedSeat_.isEmpty() || showtimeBox_->currentIndex() < 0 ||
            customerBox_->currentIndex() < 0) {
            callbacks_.notify("Chọn suất chiếu, khách hàng và ghế trống.");
            return;
        }
        if (!confirmAction(this, "Xác nhận đặt vé",
                confirmationLabel_->text() + "\n\nTiếp tục tạo vé?"))
            return;
        int ticketId = session_.book(customerBox_->currentData().toInt(),
            showtimeBox_->currentData().toInt(), selectedSeat_.toStdString(),
            methodBox_->currentIndex() == 0);
        if (!ticketId) { callbacks_.notify("Không thể đặt ghế này."); return; }
        const bool paid = session_.tickets().back().getStatus() == TicketStatus::Paid;
        callbacks_.notify(QString("Đã tạo vé #%1 — %2. Xem chi tiết ở trang Vé.")
            .arg(ticketId).arg(paid ? "Đã thanh toán demo" : "Đang chờ xác minh"));
        selectedSeat_.clear();
        callbacks_.changed();
        callbacks_.navigate(4);
    });
    refresh();
}

void SeatMapPage::selectShowtime(int showtimeId) {
    const int index = showtimeBox_->findData(showtimeId);
    if (index < 0) return;
    selectedSeat_.clear();
    showtimeBox_->setCurrentIndex(index);
    drawSeats();
}

void SeatMapPage::refresh() {
    selectedSeat_.clear();
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
    if (!show) { roomLabel_->setText("Chưa có suất chiếu."); updateConfirmation(); return; }
    const CinemaRoom* room = nullptr;
    for (const CinemaRoom& candidate : session_.rooms())
        if (candidate.getId() == show->getRoomId()) room = &candidate;
    if (!room) { roomLabel_->setText("Phòng chiếu không tồn tại."); updateConfirmation(); return; }
    roomLabel_->setText(QString("%1  •  %2")
        .arg(QString::fromStdString(room->getName()))
        .arg(QString::fromStdString(show->getStartTime())));
    for (const Seat& seat : room->getSeats()) {
        const QString id = QString::fromStdString(seat.getSeatId());
        auto* button = new QPushButton(id, this);
        button->setObjectName("seat");
        const SeatStatus status = session_.seatStatus(show->getId(), seat.getSeatId());
        const bool available = status == SeatStatus::Available;
        button->setProperty("seatState", status == SeatStatus::Held ? "held" :
            status == SeatStatus::Booked ? "booked" :
            id == selectedSeat_ ? "selected" : "available");
        button->setEnabled(available);
        button->setToolTip(QString("Ghế %1 — %2").arg(id,
            status == SeatStatus::Held ? "Đang giữ" :
            status == SeatStatus::Booked ? "Đã đặt" :
            id == selectedSeat_ ? "Đang chọn" : "Còn trống"));
        button->setCheckable(true);
        button->setChecked(id == selectedSeat_);
        button->setFixedSize(72, 54);
        connect(button, &QPushButton::clicked, this, [this, id] {
            selectedSeat_ = id;
            drawSeats();
        });
        seatGrid_->addWidget(button, seat.getRow() - 'A', seat.getNumber() - 1);
    }
    updateConfirmation();
}

void SeatMapPage::updateConfirmation() {
    const Showtime* show = session_.schedule().findShowtime(showtimeBox_->currentData().toInt());
    const Customer* customer = session_.users().findCustomer(customerBox_->currentData().toInt());
    const Movie* movie = show ? session_.movies().findMovie(show->getMovieId()) : nullptr;
    const bool ready = show && customer && movie && !selectedSeat_.isEmpty() &&
        session_.seatStatus(show->getId(), selectedSeat_.toStdString()) == SeatStatus::Available;
    bookButton_->setEnabled(ready);
    if (!ready) {
        confirmationLabel_->setText("Chọn suất, khách hàng và ghế trống để xem thông tin vé.");
        return;
    }
    confirmationLabel_->setText(QString("XÁC NHẬN  •  %1  •  %2  •  Ghế %3  •  %4  •  %5")
        .arg(QString::fromStdString(movie->getTitle()))
        .arg(QString::fromStdString(show->getStartTime()))
        .arg(selectedSeat_)
        .arg(QString::fromStdString(customer->getName()))
        .arg(vnd(show->getPriceVnd())));
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
    if (session_.users().getCustomers().empty())
        customerList_->addWidget(mutedLabel("Chưa có khách hàng. Hãy đăng ký để đặt vé.", this));
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
        details->addWidget(mutedLabel(QString("Khách #%1  •  Suất #%2  •  %3")
            .arg(ticket.getCustomerId()).arg(ticket.getShowtimeId())
            .arg(vnd(ticket.getPriceVnd())), frame));
        row->addLayout(details, 1);
        const char* status = ticket.getStatus() == TicketStatus::Paid ? "Đã thanh toán" :
                             ticket.getStatus() == TicketStatus::Pending ? "Đang chờ" : "Đã hủy";
        auto* badge = new QLabel(status, frame);
        badge->setObjectName("badge");
        row->addWidget(badge);
        if (ticket.getStatus() != TicketStatus::Cancelled) {
            auto* cancel = new QPushButton("Hủy vé", frame);
            int id = ticket.getId();
            connect(cancel, &QPushButton::clicked, this, [this, id] {
                if (!confirmAction(this, "Xác nhận hủy vé",
                        QString("Hủy vé #%1 và trả ghế về trạng thái trống?").arg(id)))
                    return;
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
    payButton_ = new QPushButton("Xử lý", form);
    payButton_->setObjectName("primary");
    row->addWidget(payButton_);
    root->addWidget(form);
    root->addStretch();
    connect(payButton_, &QPushButton::clicked, this, [this] {
        if (ticketBox_->currentIndex() < 0) {
            callbacks_.notify("Không có vé Pending để thanh toán."); return;
        }
        if (!confirmAction(this, "Xác nhận thanh toán mô phỏng",
                QString("Xử lý vé #%1 bằng %2?").arg(ticketBox_->currentData().toInt())
                    .arg(methodBox_->currentText())))
            return;
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
            ticketBox_->addItem(QString("Vé #%1  •  Ghế %2  •  %3")
                .arg(ticket.getId()).arg(QString::fromStdString(ticket.getSeatId()))
                .arg(vnd(ticket.getPriceVnd())), ticket.getId());
    }
    payButton_->setEnabled(ticketBox_->count() > 0);
}
