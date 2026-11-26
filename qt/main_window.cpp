#include "main_window.h"
#include "ui_helpers.h"
#include "ui_dialogs.h"
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QStandardPaths>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : MainWindow(QString(), parent) {}

MainWindow::MainWindow(const QString& dataDirectory, QWidget* parent)
    : QMainWindow(parent),
      dataDirectory_(dataDirectory.isEmpty() ?
          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) :
          dataDirectory) {
    setObjectName("mainWindow");
    setWindowTitle("OOP Cinema · Demo V1");
    resize(1120, 760);
    setMinimumSize(860, 620);
    setStyleSheet(R"QSS(
        QMainWindow, QWidget { background: #0c111d; color: #f0f3f8;
            font-family: "Segoe UI", "Arial"; font-size: 14px; }
        QLabel { background: transparent; }
        QFrame#sidebar { background: #111a29; border-right: 1px solid #263447; }
        QFrame#topbar { background: #0c111d; border-bottom: 1px solid #263447; }
        QFrame#card { background: #162236; border: 1px solid #31445c;
            border-radius: 15px; }
        QFrame#hero { background: #352038; border: 1px solid #7e3449;
            border-radius: 18px; }
        QLabel#brand { color: #ff6371; font-size: 22px; font-weight: 800; }
        QLabel#pageTitle { font-size: 29px; font-weight: 800; }
        QLabel#heroTitle { font-size: 26px; font-weight: 800; }
        QLabel#sectionTitle { color: #ffb5ba; font-size: 12px;
            font-weight: 800; letter-spacing: 2px; }
        QLabel#muted { color: #a7b6c8; }
        QLabel#dataPath { color: #c3d0df; }
        QLabel#cardTitle { font-size: 17px; font-weight: 700; }
        QLabel#poster { background: #423046; color: #ffb6be;
            border-radius: 10px; font-size: 26px; font-weight: 800; }
        QLabel#showTime { color: #ff818d; font-size: 21px; font-weight: 800; }
        QLabel#screen { color: #9fc6dc; background: #2b4157;
            border-radius: 6px; padding: 10px; margin-bottom: 16px; }
        QLabel#badge { color: #ffc3c8; background: #563044;
            border-radius: 8px; padding: 7px 12px; }
        QLabel#toast { color: #b6e6d1; }
        QFrame#storageBar { background: #121c2a; border-bottom: 1px solid #263447; }
        QLabel#saveStatus[state="dirty"] { color: #ffd08a; }
        QLabel#saveStatus[state="saved"] { color: #8cdec1; }
        QLabel#saveStatus[state="error"] { color: #ff8d98; }
        QPushButton { background: #25354a; color: #ecf4fa; border: 1px solid #40536a;
            border-radius: 10px; padding: 10px 15px; font-weight: 600; }
        QPushButton:hover { background: #334862; }
        QPushButton:disabled { background: #1c2838; color: #78899b;
            border-color: #2b3b4e; }
        QPushButton:focus { border: 2px solid #90c8f0; }
        QPushButton#primary, QPushButton#bookButton { background: #e64d62;
            border: 1px solid #e64d62; color: white; }
        QPushButton#primary:hover, QPushButton#bookButton:hover { background: #f4697a; }
        QPushButton#primary:disabled, QPushButton#bookButton:disabled {
            background: #613741; color: #a9a7ae; border-color: #613741; }
        QPushButton#nav { background: transparent; color: #a7b6c8;
            border: 0; text-align: left; padding: 13px 17px; }
        QPushButton#nav:hover { background: #24334a; color: white; }
        QPushButton#nav[active="true"] { background: #3a2a3d; color: #ffadb5;
            border-left: 3px solid #ff6677; }
        QPushButton#seat[seatState="available"] { background: #294b50;
            border: 1px solid #57a89c; }
        QPushButton#seat[seatState="held"] { background: #634b35;
            border: 1px solid #c89b5c; color: #ffe0b0; }
        QPushButton#seat[seatState="booked"] { background: #533744;
            border: 1px solid #a56c78; color: #efc4ce; }
        QPushButton#seat[seatState="selected"] { background: #e64d62;
            border: 1px solid #ff9fa9; color: white; }
        QPushButton#seat:disabled { font-weight: 700; }
        QLabel[legendState="available"] { color: #70c9b8; }
        QLabel[legendState="held"] { color: #eac18e; }
        QLabel[legendState="booked"] { color: #e6a8b7; }
        QLabel[legendState="selected"] { color: #ff818d; }
        QComboBox, QLineEdit { background: #0e1928; color: #f0f3f8;
            border: 1px solid #41536a; border-radius: 9px; padding: 9px; }
        QComboBox:focus, QLineEdit:focus { border: 2px solid #90c8f0; }
        QComboBox QAbstractItemView { background: #162236; color: #f0f3f8;
            selection-background-color: #ad455d; }
        QScrollArea { border: 0; }
        QScrollBar:vertical { background: #111a29; width: 10px; }
        QScrollBar::handle:vertical { background: #40536a; border-radius: 5px; }
        QScrollBar::handle:vertical:hover { background: #708ba8; }
    )QSS");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* shell = new QHBoxLayout(central);
    shell->setContentsMargins(0, 0, 0, 0);
    shell->setSpacing(0);
    auto* sidebar = new QFrame(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(222);
    auto* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(15, 26, 15, 22);
    side->setSpacing(6);
    auto* brand = new QLabel("◉ CINEMA", sidebar);
    brand->setObjectName("brand");
    side->addWidget(brand);
    side->addWidget(mutedLabel("BOOKING  /  DEMO", sidebar));
    side->addSpacing(30);
    const QStringList labels = {"⌂  Trang chủ", "▤  Suất chiếu", "▦  Chọn ghế",
                                "♙  Khách hàng", "▣  Vé của bạn", "◇  Thanh toán"};
    for (int i = 0; i < labels.size(); ++i) {
        auto* button = new QPushButton(labels[i], sidebar);
        button->setObjectName("nav");
        button->setProperty("active", false);
        button->setCursor(Qt::PointingHandCursor);
        side->addWidget(button);
        navigation_.push_back(button);
        connect(button, &QPushButton::clicked, this, [this, i] {
            if (i == 1) showtimes_->setMovieFilter(0);
            showPage(i);
        });
    }
    side->addStretch();
    auto* foot = mutedLabel("OOP CINEMA\nQt 6 · Bản demo V1", sidebar);
    side->addWidget(foot);
    shell->addWidget(sidebar);

    auto* content = new QWidget(central);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    auto* topbar = new QFrame(content);
    topbar->setObjectName("topbar");
    auto* top = new QHBoxLayout(topbar);
    top->setContentsMargins(36, 14, 36, 14);
    top->addWidget(mutedLabel("ĐẶT VÉ XEM PHIM", topbar));
    top->addStretch();
    toast_ = new QLabel("Sẵn sàng", topbar);
    toast_->setObjectName("toast");
    toast_->setWordWrap(true);
    top->addWidget(toast_);
    contentLayout->addWidget(topbar);
    storageBar_ = new StorageBar(
        [this] {
            if (loadFailed_ && !confirmAction(this, "Bản lưu đang lỗi",
                "Thư mục này có bản lưu lỗi. Bạn có chắc muốn ghi đè bằng dữ liệu đang mở?"))
                return;
            saveData();
        },
        [this] {
            if (!confirmDiscardChanges("tải bản lưu khác")) return;
            loadData();
        },
        [this] { chooseDirectory(); }, content);
    storageBar_->setDirectory(dataDirectory_);
    storageBar_->setStatus("", session_.dirty());
    contentLayout->addWidget(storageBar_);
    pages_ = new QStackedWidget(content);
    UiCallbacks callbacks{
        [this](int index) { showPage(index); },
        [this](int movieId) { showtimes_->setMovieFilter(movieId); showPage(1); },
        [this](int showtimeId) { seats_->selectShowtime(showtimeId); showPage(2); },
        [this](const QString& text) { toast_->setText(text); },
        [this] { refreshAll(); storageBar_->setStatus("", session_.dirty()); }
    };
    home_ = new HomePage(session_, callbacks, pages_);
    showtimes_ = new ShowtimePage(session_, callbacks, pages_);
    seats_ = new SeatMapPage(session_, callbacks, pages_);
    customers_ = new CustomerPage(session_, callbacks, pages_);
    tickets_ = new TicketPage(session_, callbacks, pages_);
    payments_ = new PaymentPage(session_, callbacks, pages_);
    QWidget* pageList[] = {home_, showtimes_, seats_, customers_, tickets_, payments_};
    for (QWidget* page : pageList) {
        auto* scroll = new QScrollArea(pages_);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidget(page);
        pages_->addWidget(scroll);
    }
    contentLayout->addWidget(pages_, 1);
    shell->addWidget(content, 1);
    showPage(0);
    const QDir savedDirectory(dataDirectory_);
    const QStringList dataFiles = {"movies.txt", "rooms.txt", "showtimes.txt",
                                   "customers.txt", "tickets.txt"};
    bool hasSavedFile = false;
    for (const QString& file : dataFiles)
        if (savedDirectory.exists(file)) hasSavedFile = true;
    if (hasSavedFile) loadData();
}

void MainWindow::refreshAll() {
    home_->refresh();
    showtimes_->refresh();
    seats_->refresh();
    customers_->refresh();
    tickets_->refresh();
    payments_->refresh();
}

void MainWindow::showPage(int index) {
    if (index < 0 || index >= pages_->count()) return;
    pages_->setCurrentIndex(index);
    for (int i = 0; i < static_cast<int>(navigation_.size()); ++i) {
        navigation_[i]->setProperty("active", i == index);
        navigation_[i]->style()->unpolish(navigation_[i]);
        navigation_[i]->style()->polish(navigation_[i]);
    }
}

int MainWindow::currentPage() const { return pages_->currentIndex(); }

bool MainWindow::saveData() {
    const bool ok = session_.save(dataDirectory_.toUtf8().toStdString());
    if (ok) {
        loadFailed_ = false;
        toast_->setText("Đã lưu phim, phòng, suất, khách và vé.");
        storageBar_->setStatus("", false);
    } else {
        toast_->setText("Lưu thất bại: " + QString::fromStdString(session_.lastError()));
        storageBar_->setStatus("Lưu thất bại", session_.dirty(), true);
    }
    return ok;
}

bool MainWindow::loadData() {
    const bool ok = session_.load(dataDirectory_.toUtf8().toStdString());
    if (ok) {
        loadFailed_ = false;
        refreshAll();
        toast_->setText("Đã khôi phục bản lưu hợp lệ.");
        storageBar_->setStatus("", false);
    } else {
        loadFailed_ = true;
        toast_->setText("Bản lưu lỗi: " + QString::fromStdString(session_.lastError()) +
                        ". Dữ liệu đang mở được giữ nguyên.");
        storageBar_->setStatus("Bản lưu lỗi", session_.dirty(), true);
    }
    return ok;
}

void MainWindow::chooseDirectory() {
    const QString chosen = QFileDialog::getExistingDirectory(this, "Chọn thư mục dữ liệu",
                                                            dataDirectory_);
    if (chosen.isEmpty() || chosen == dataDirectory_ ||
        !confirmDiscardChanges("chuyển thư mục dữ liệu")) return;
    dataDirectory_ = chosen;
    storageBar_->setDirectory(dataDirectory_);
    toast_->setText("Đã chọn thư mục. Chọn Tải hoặc Lưu để áp dụng.");
    storageBar_->setStatus("", session_.dirty());
    loadFailed_ = false;
}

bool MainWindow::confirmDiscardChanges(const QString& action) {
    if (!session_.dirty()) return true;
    const auto choice = askUnsavedChanges(this, action);
    if (choice == QMessageBox::Cancel) return false;
    if (choice == QMessageBox::Save) return saveData();
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (confirmDiscardChanges("thoát chương trình")) event->accept();
    else event->ignore();
}
