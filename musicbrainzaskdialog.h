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

#ifndef MUSICBRAINZASKDIALOG_H
#define MUSICBRAINZASKDIALOG_H

#include <QDialog>

namespace Ui {
class MusicBrainzAskDialog;
}

class MusicBrainzAskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MusicBrainzAskDialog(QWidget *parent = nullptr);
    ~MusicBrainzAskDialog();

    static bool showAskDialog(QWidget *owner);
    static bool downloadOkMaybeAsk(QWidget *parentWindow);
    static bool downloadOk(void);

private slots:
    void on_yesBtn_clicked();

    void on_noBtn_clicked();

private:
    Ui::MusicBrainzAskDialog *ui;
};

#endif // MUSICBRAINZASKDIALOG_H
