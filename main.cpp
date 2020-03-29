#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    a.setOrganizationName(QStringLiteral("Genosse Einhorn"));
    a.setOrganizationDomain(QStringLiteral("genosse-einhorn.de"));
    a.setApplicationName(QStringLiteral("KuemCddaExtract"));
    a.setApplicationDisplayName(QStringLiteral(u"Genosse Einhorn’s CD Audio Extractor"));
    a.setApplicationVersion(QStringLiteral("v2020.03.0"));

    MainWindow w;
    w.show();

    return a.exec();
}
