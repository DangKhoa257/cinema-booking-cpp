#pragma once

#include <QMessageBox>
#include <QPushButton>

inline bool confirmAction(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox box(QMessageBox::Question, title, text,
                    QMessageBox::Yes | QMessageBox::No, parent);
    box.setDefaultButton(QMessageBox::No);
    box.button(QMessageBox::Yes)->setText("Đồng ý");
    box.button(QMessageBox::No)->setText("Không");
    return box.exec() == QMessageBox::Yes;
}

inline QMessageBox::StandardButton askUnsavedChanges(QWidget* parent,
                                                      const QString& action) {
    QMessageBox box(QMessageBox::Warning, "Thay đổi chưa lưu",
        "Bạn muốn lưu thay đổi trước khi " + action + "?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, parent);
    box.setDefaultButton(QMessageBox::Save);
    box.button(QMessageBox::Save)->setText("Lưu");
    box.button(QMessageBox::Discard)->setText("Không lưu");
    box.button(QMessageBox::Cancel)->setText("Hủy");
    return static_cast<QMessageBox::StandardButton>(box.exec());
}
