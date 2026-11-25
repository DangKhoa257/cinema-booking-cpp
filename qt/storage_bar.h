#pragma once

#include <QFrame>
#include <functional>

class QLabel;

class StorageBar : public QFrame {
public:
    StorageBar(std::function<void()> save, std::function<void()> load,
               std::function<void()> choose, QWidget* parent = nullptr);
    void setDirectory(const QString& path);
    void setStatus(const QString& message, bool dirty, bool error = false);

private:
    QLabel* pathLabel_;
    QLabel* statusLabel_;
};
