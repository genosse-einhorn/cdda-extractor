#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    a.setOrganizationName(QStringLiteral("Genosse Einhorn"));
    a.setOrganizationDomain(QStringLiteral("genosse-einhorn.de"));
    a.setApplicationName(QStringLiteral("KuemCddaExtract"));

    MainWindow w;
    w.show();

    return a.exec();
}
