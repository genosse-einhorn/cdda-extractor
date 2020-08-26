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

#include "musicbrainzprivacyinfodialog.h"
#include "ui_musicbrainzprivacyinfodialog.h"

#include <QSettings>

MusicBrainzPrivacyInfoDialog::MusicBrainzPrivacyInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MusicBrainzPrivacyInfoDialog)
{
    ui->setupUi(this);
}

MusicBrainzPrivacyInfoDialog::~MusicBrainzPrivacyInfoDialog()
{
    delete ui;
}

bool MusicBrainzPrivacyInfoDialog::showInfoDialog(QWidget *owner)
{
    MusicBrainzPrivacyInfoDialog dialog(owner);

    return dialog.exec() > 0;
}

bool MusicBrainzPrivacyInfoDialog::maybeShowInfoDialog(QWidget *parentWindow)
{
    QSettings settings;

    int val = settings.value(QStringLiteral("MusicBrainz Skip Privacy Info"), 0).toInt();

    if (val > 0) {
        return true;
    } else {
        return showInfoDialog(parentWindow);
    }
}

void MusicBrainzPrivacyInfoDialog::on_yesBtn_clicked()
{
    QSettings settings;
    settings.setValue(QStringLiteral("MusicBrainz Skip Privacy Info"), ui->cbDontAskAgain->isChecked() ? 1 : 0);

    done(1);
}

void MusicBrainzPrivacyInfoDialog::on_noBtn_clicked()
{
    done(0);
}
