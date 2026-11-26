#include "v2_window.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    const QIcon appIcon(":/oop-cinema/icon.png");
    app.setWindowIcon(appIcon);
    QCoreApplication::setOrganizationName("OOP Cinema");
    QCoreApplication::setApplicationName("OOP Cinema 2.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("OOP Cinema 2.0 — đặt vé demo");
    parser.addHelpOption();
    parser.addOption({"data-dir", "Thư mục bản lưu V2", "directory"});
    parser.addOption({"window-size", "Kích thước cửa sổ: widthxheight", "size"});
    parser.process(app);
    const QString base = QCoreApplication::applicationDirPath();
    const QString data = QDir(base).filePath("v2-data");
    QString storage = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (storage.isEmpty()) storage = QDir::home().filePath("OOPCinema");
    if (parser.isSet("data-dir")) storage = parser.value("data-dir");
    V2Window window(QDir(data).filePath("movie_details.txt"),
                    QDir(data).filePath("cinema_demo.txt"),
                    QDir(base).filePath("assets"), storage);
    window.setWindowIcon(appIcon);
    if (parser.isSet("window-size")) {
        const QStringList parts = parser.value("window-size").split('x');
        if (parts.size() == 2) window.resize(parts[0].toInt(), parts[1].toInt());
    }
    window.show();
    return app.exec();
}
