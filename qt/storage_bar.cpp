#include "storage_bar.h"
#include "ui_helpers.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QPushButton>
#include <QStyle>
#include <utility>

StorageBar::StorageBar(std::function<void()> save, std::function<void()> load,
                       std::function<void()> choose, QWidget* parent)
    : QFrame(parent) {
    setObjectName("storageBar");
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(36, 10, 36, 10);
    row->setSpacing(10);
    pathLabel_ = mutedLabel("", this);
    pathLabel_->setObjectName("dataPath");
    pathLabel_->setMinimumWidth(100);
    pathLabel_->setMaximumWidth(240);
    row->addWidget(pathLabel_, 1);
    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName("saveStatus");
    row->addWidget(statusLabel_);
    auto* chooseButton = new QPushButton("Thư mục…", this);
    chooseButton->setObjectName("chooseDataFolder");
    row->addWidget(chooseButton);
    auto* loadButton = new QPushButton("Tải", this);
    loadButton->setObjectName("loadDataButton");
    row->addWidget(loadButton);
    auto* saveButton = new QPushButton("Lưu", this);
    saveButton->setObjectName("saveDataButton");
    row->addWidget(saveButton);
    connect(chooseButton, &QPushButton::clicked, this, std::move(choose));
    connect(loadButton, &QPushButton::clicked, this, std::move(load));
    connect(saveButton, &QPushButton::clicked, this, std::move(save));
}

void StorageBar::setDirectory(const QString& path) {
    pathLabel_->setText("Dữ liệu: " + QFileInfo(path).fileName());
    pathLabel_->setToolTip("Thư mục dữ liệu: " + path);
}

void StorageBar::setStatus(const QString& message, bool dirty, bool error) {
    statusLabel_->setText(message.isEmpty() ? (dirty ? "● Chưa lưu" : "✓ Đã lưu") : message);
    statusLabel_->setProperty("state", error ? "error" : dirty ? "dirty" : "saved");
    statusLabel_->style()->unpolish(statusLabel_);
    statusLabel_->style()->polish(statusLabel_);
}
