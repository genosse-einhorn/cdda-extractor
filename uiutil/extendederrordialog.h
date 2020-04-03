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

#ifndef EXTENDEDERRORDIALOG_H
#define EXTENDEDERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ExtendedErrorDialog;
}

class ExtendedErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtendedErrorDialog(QWidget *parent = 0);
    ~ExtendedErrorDialog();

    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString details READ details WRITE setDetails NOTIFY detailsChanged)

    QString message() const;
    QString details() const;

    static void show(QWidget *parent, const QString &message, const QString &details);
signals:
    void messageChanged(const QString &newMessage);
    void detailsChanged(const QString &newDetails);

public slots:
    void setMessage(const QString &message);
    void setDetails(const QString &details);

private:
    Ui::ExtendedErrorDialog *ui;
};

#endif // EXTENDEDERRORDIALOG_H
