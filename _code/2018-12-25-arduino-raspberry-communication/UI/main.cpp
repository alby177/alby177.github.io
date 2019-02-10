#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("AlbyHomeDev");
    QCoreApplication::setOrganizationDomain("albyhome.com");
    QCoreApplication::setApplicationName("RasPi Client");

    MainWindow w;
    w.show();

    return a.exec();
}
