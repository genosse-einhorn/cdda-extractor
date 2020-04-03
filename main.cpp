// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
