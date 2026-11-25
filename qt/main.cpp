#include "main_window.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("CinemaBooking");
    app.setApplicationName("CinemaBooking");
    MainWindow window;
    window.show();
    return app.exec();
}
