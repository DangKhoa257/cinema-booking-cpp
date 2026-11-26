#include "v2_window.h"
#include "v2_widgets.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        QTemporaryDir temp;
        V2Window window(QString::fromUtf8(MOVIE_DATA_FILE),
                        QString::fromUtf8(CINEMA_DATA_FILE),
                        QString::fromUtf8(ASSET_DIRECTORY), temp.path());
        require(window.session().ready(), "window data ready");
        const auto featured = window.session().featuredMovies();
        require(featured.size() == 9, "nine movies with posters");
        QFile mapping(QDir(QString::fromUtf8(ASSET_DIRECTORY)).filePath("posters/mapping.json"));
        require(mapping.open(QIODevice::ReadOnly), "poster mapping exists");
        const QJsonDocument mappingDoc = QJsonDocument::fromJson(mapping.readAll());
        require(mappingDoc.isArray() && mappingDoc.array().size() == 9, "nine mapped posters");
        for (const auto& entry : mappingDoc.array()) {
            const QJsonObject row = entry.toObject();
            const auto* details = window.session().movies().find(row.value("movie_id").toInt());
            require(details && window.session().isFeaturedMovie(details->movieId), "mapped movie id");
            require(QString::fromStdString(details->title) == row.value("catalog_title").toString(),
                    "mapped movie title");
            require(QString::fromStdString(details->posterAsset) ==
                    "posters/" + row.value("file").toString(), "mapped poster file");
            const QImage poster(posterFile(QString::fromUtf8(ASSET_DIRECTORY), *details));
            require(!poster.isNull() && qAbs(poster.width() * 3 - poster.height() * 2) < 5,
                    "valid 2:3 poster image");
        }
        window.resize(860, 620);
        window.show();
        app.processEvents();
        auto* banner = static_cast<MovieBanner*>(window.findChild<QWidget*>("movieBanner"));
        require(banner && banner->height() == 285 &&
                banner->currentMovieId() == featured.front()->movieId,
                "first home banner matches catalog");
        for (size_t index = 1; index < featured.size(); ++index) {
            auto* next = banner->findChild<QPushButton*>("bannerNext");
            require(next, "banner next button");
            next->click();
            app.processEvents();
            auto* title = banner->findChild<QLabel*>("bannerTitle");
            auto* poster = banner->findChild<QLabel*>("bannerPoster");
            require(banner->currentMovieId() == featured[index]->movieId && title &&
                    title->text() == QString::fromStdString(featured[index]->title) &&
                    poster && !poster->pixmap().isNull(), "banner metadata and poster update");
        }
        banner->findChild<QPushButton*>("bannerNext")->click();
        require(banner->currentMovieId() == featured.front()->movieId, "banner wraps around");
        require(!window.findChild<QPushButton*>("movie-104") &&
                !window.findChild<QPushButton*>("movie-108"), "historical movies hidden from cards");
        auto* movie = window.findChild<QPushButton*>("movie-101");
        require(movie, "first movie card exists");
        movie->click();
        app.processEvents();
        auto* show = window.findChild<QPushButton*>("showtime-1");
        require(show, "correct movie and branch showtime");
        show->click();
        app.processEvents();
        auto* seat = window.findChild<QPushButton*>("seat-A1");
        require(seat && seat->isEnabled(), "available physical seat");
        seat->click();
        auto* map = static_cast<SeatMapWidget*>(window.findChild<QWidget*>("seatMap"));
        require(map && map->selected().count("A1") == 1, "selection flows to seat map");
        auto* openSnacks = window.findChild<QPushButton*>("openSnacks");
        require(openSnacks && openSnacks->isEnabled(), "snack step is available");
        openSnacks->click();
        app.processEvents();
        require(window.findChildren<QFrame*>("snackCard").size() == 5,
                "exactly five horizontal snack cards");
        require(window.findChildren<QSpinBox*>().isEmpty(), "no spinbox quantity arrows");
        for (const auto& item : window.session().snacks()) {
            const QString id = QString::number(item.id);
            auto* photo = window.findChild<QLabel*>("snack-image-" + id);
            auto* name = window.findChild<QLabel*>("snack-name-" + id);
            auto* contents = window.findChild<QLabel*>("snack-contents-" + id);
            auto* price = window.findChild<QLabel*>("snack-price-" + id);
            auto* minus = window.findChild<QPushButton*>("snack-minus-" + id);
            auto* plus = window.findChild<QPushButton*>("snack-plus-" + id);
            const QImage product(QDir(QString::fromUtf8(ASSET_DIRECTORY)).filePath(item.imageFile));
            require(photo && !photo->pixmap().isNull() && product.width() >= 300 &&
                    product.height() >= 300 && name && name->text() == item.name &&
                    contents && contents->text() == item.contents &&
                    price && price->text() == vnd(item.price) &&
                    minus && !minus->isEnabled() && plus && plus->isEnabled(),
                    "snack photo, metadata, price and initial controls");
        }
        auto* plusPopcorn = window.findChild<QPushButton*>("snack-plus-1");
        auto* minusPopcorn = window.findChild<QPushButton*>("snack-minus-1");
        auto* plusCombo = window.findChild<QPushButton*>("snack-plus-3");
        plusPopcorn->click(); plusPopcorn->click(); plusCombo->click();
        auto* snackTotal = window.findChild<QLabel*>("snack-total");
        require(window.findChild<QLabel*>("snack-count-1")->text() == "2" &&
                snackTotal && snackTotal->text().contains("259.000"),
                "plus updates quantities and order total");
        minusPopcorn->click();
        require(window.findChild<QLabel*>("snack-count-1")->text() == "1" &&
                snackTotal->text().contains("214.000"), "minus updates order total");
        window.findChild<QPushButton*>("openCheckout")->click();
        app.processEvents();
        window.findChild<QPushButton*>("backToSnacks")->click();
        app.processEvents();
        require(window.findChild<QLabel*>("snack-count-1")->text() == "1" &&
                window.findChild<QLabel*>("snack-count-3")->text() == "1" &&
                window.findChild<QLabel*>("snack-total")->text().contains("214.000"),
                "snack selection survives page navigation");
        for (auto* button : window.findChildren<QPushButton*>())
            require(!button->text().contains("Lưu dữ liệu"), "no manual save button");
        window.resize(1120, 760);
        app.processEvents();
        require(window.isVisible(), "resized GUI stays visible");
        std::cout << "V2 Qt navigation passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}
