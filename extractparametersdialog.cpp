#include "extractparametersdialog.h"
#include "ui_extractparametersdialog.h"
#include "encoder/lame_backend.h"
#include "uiutil/win32iconloader.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>
#include <QIcon>

ExtractParametersDialog::ExtractParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtractParametersDialog)
{
    ui->setupUi(this);

    ui->eOutputDirectory->setText(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (Encoder::LameFuncTable::get())
    {
        ui->lMp3->setEnabled(true);
        ui->rbMp3->setEnabled(true);
    }
    else
    {
        ui->lMp3->setText(tr("%1<br><font color=red>Not available, please install <code>libmp3lame</code></font>").arg(ui->lMp3->text()));
    }

#ifdef Q_OS_WIN32
    QIcon themeFolder = IconLoader::fromShellStock(SIID_FOLDER);
#else
    QIcon themeFolder = QIcon::fromTheme(QStringLiteral("folder"));
#endif
    if (!themeFolder.isNull())
        ui->iwFolder->setIcon(themeFolder);
    else
        ui->iwFolder->setIcon(QIcon(QStringLiteral(":/inode-directory.svg")));

#ifdef Q_OS_WIN32
    QIcon themeDrive = IconLoader::fromShellStock(SIID_DRIVECD);
#else
    QIcon themeDrive = QIcon::fromTheme(QStringLiteral("drive-optical"));
#endif
    if (!themeDrive.isNull())
        ui->iwCd->setIcon(themeDrive);
    else
        ui->iwCd->setIcon(QIcon(QStringLiteral(":/drive-cdrom.svg")));

#ifdef Q_OS_WIN32
    QIcon themeFormat = IconLoader::fromShellStock(SIID_AUDIOFILES);
#else
    QIcon themeFormat = QIcon::fromTheme(QStringLiteral("audio-x-generic"));
#endif
    if (!themeFormat.isNull())
        ui->iwFileFormat->setIcon(themeFormat);
    else
        ui->iwFileFormat->setIcon(QIcon(QStringLiteral(":/audio-x-generic.svg")));

    ui->iwFolder->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->iwCd->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->iwFileFormat->setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

ExtractParametersDialog::~ExtractParametersDialog()
{
    delete ui;
}

void ExtractParametersDialog::setAlbumInfo(const QString &title, const QString &artist)
{
    m_album = title;
    m_artist = artist;
    ui->cbCreateSubfolder->setText(tr("Create Subfolder: %1/%2").arg(artist).arg(title));
}

QString ExtractParametersDialog::outputDir() const
{
    QString dir = ui->eOutputDirectory->text();
    if (ui->cbCreateSubfolder->isChecked())
        dir = QStringLiteral("%1/%2/%3").arg(dir, m_artist, m_album);

    return dir;
}

QString ExtractParametersDialog::format() const
{
    if (ui->rbFlac->isChecked())
        return QStringLiteral("flac");
    if (ui->rbMp3->isChecked())
        return QStringLiteral("mp3");

    return QStringLiteral("wav");
}

bool ExtractParametersDialog::paranoiaActivated() const
{
    return ui->rbParanoiaExtract->isChecked();
}

void ExtractParametersDialog::on_bBrowseDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select output directory"),
                                                    ui->eOutputDirectory->text());
    if (dir.size())
        ui->eOutputDirectory->setText(dir);
}


void ExtractParametersDialog::showEvent(QShowEvent *event)
{
    resize(width(), heightForWidth(width()));

    QDialog::showEvent(event);
}
