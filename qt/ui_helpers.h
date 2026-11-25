#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

inline QLabel* mutedLabel(const QString& text, QWidget* parent = nullptr) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("muted");
    label->setWordWrap(true);
    return label;
}

inline QFrame* card(QWidget* parent = nullptr) {
    auto* frame = new QFrame(parent);
    frame->setObjectName("card");
    return frame;
}

inline void clearLayout(QLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) { item->widget()->hide(); item->widget()->deleteLater(); }
        delete item;
    }
}

inline QVBoxLayout* pageLayout(QWidget* page, const QString& title,
                                const QString& subtitle) {
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(38, 32, 38, 32);
    layout->setSpacing(18);
    auto* heading = new QLabel(title, page);
    heading->setObjectName("pageTitle");
    layout->addWidget(heading);
    layout->addWidget(mutedLabel(subtitle, page));
    return layout;
}
