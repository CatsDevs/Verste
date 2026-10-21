#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Verste");
    QApplication::setOrganizationName("CatsDevs");
    MainWindow window;
    window.show();
    return app.exec();
}
