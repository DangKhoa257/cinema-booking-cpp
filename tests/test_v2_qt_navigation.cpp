#include "v2_window.h"
#include "v2_widgets.h"

#include <QApplication>
#include <QDir>
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
        window.resize(860, 620);
        window.show();
        app.processEvents();
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
