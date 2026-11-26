#include "v2_window.h"
#include "v2_widgets.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
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
