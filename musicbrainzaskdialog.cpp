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

    connect(ui->noBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->yesBtn, &QPushButton::clicked, this, &QDialog::accept);
}

MusicBrainzAskDialog::~MusicBrainzAskDialog()
{
    delete ui;
}

bool MusicBrainzAskDialog::showAskDialog(QWidget *owner)
{
    MusicBrainzAskDialog dialog(owner);

    QSettings settings;

    if (dialog.exec() == QDialog::Accepted) {
        settings.setValue(QStringLiteral("MusicBrainz Auto Download"), int(MUSICBRAINZ_DOWNLOAD_YES));
        return true;
    } else {
        settings.setValue(QStringLiteral("MusicBrainz Auto Download"), int(MUSICBRAINZ_DOWNLOAD_NO));
        return false;
    }
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
