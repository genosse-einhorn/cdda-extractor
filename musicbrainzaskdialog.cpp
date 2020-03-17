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
