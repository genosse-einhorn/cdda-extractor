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

#include "musicbrainzaskdialog.h"
#include "ui_musicbrainzaskdialog.h"

#include <QSettings>

enum SettingsValue {
    MUSICBRAINZ_DOWNLOAD_UNDECIDED = 0,
    MUSICBRAINZ_DOWNLOAD_NO = 1,
    MUSICBRAINZ_DOWNLOAD_YES = 2
};

MusicBrainzAskDialog::MusicBrainzAskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MusicBrainzAskDialog)
{
    ui->setupUi(this);
}

MusicBrainzAskDialog::~MusicBrainzAskDialog()
{
    delete ui;
}

bool MusicBrainzAskDialog::showAskDialog(QWidget *owner)
{
    MusicBrainzAskDialog dialog(owner);

    return dialog.exec() == MUSICBRAINZ_DOWNLOAD_YES;
}

bool MusicBrainzAskDialog::downloadOkMaybeAsk(QWidget *parentWindow)
{
    QSettings settings;

    int val = settings.value(QStringLiteral("MusicBrainz Auto Download"), int(MUSICBRAINZ_DOWNLOAD_UNDECIDED)).toInt();

    if (val == MUSICBRAINZ_DOWNLOAD_UNDECIDED) {
        return showAskDialog(parentWindow);
    } else {
        return val == MUSICBRAINZ_DOWNLOAD_YES;
    }
}

bool MusicBrainzAskDialog::downloadOk()
{
    QSettings settings;

    int val = settings.value(QStringLiteral("MusicBrainz Auto Download"), int(MUSICBRAINZ_DOWNLOAD_UNDECIDED)).toInt();
    return val == MUSICBRAINZ_DOWNLOAD_YES;
}

void MusicBrainzAskDialog::on_yesBtn_clicked()
{
    QSettings settings;
    settings.setValue(QStringLiteral("MusicBrainz Auto Download"), int(MUSICBRAINZ_DOWNLOAD_YES));

    done(MUSICBRAINZ_DOWNLOAD_YES);
}

void MusicBrainzAskDialog::on_noBtn_clicked()
{
    QSettings settings;
    settings.setValue(QStringLiteral("MusicBrainz Auto Download"), int(MUSICBRAINZ_DOWNLOAD_NO));

    done(MUSICBRAINZ_DOWNLOAD_NO);
}
