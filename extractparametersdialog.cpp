#include "extractparametersdialog.h"
#include "ui_extractparametersdialog.h"
#include "uiutil/win32iconloader.h"
#include "fileutil.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>
#include <QIcon>
#include <QSettings>

ExtractParametersDialog::ExtractParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtractParametersDialog)
{
    ui->setupUi(this);

    ui->eOutputDirectory->setText(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

    QSettings settings;
    QString dir = settings.value(QStringLiteral("Output Directory")).toString();
    if (dir.size() && QDir(dir).exists()) {
        ui->eOutputDirectory->setText(dir);
    }
    ui->cbCreateSubfolder->setChecked(settings.value(QStringLiteral("Create Subfolder"), bool(true)).toBool());
    QString mode = settings.value(QStringLiteral("Paranoia Mode"), QStringLiteral("paranoia")).toString();
    if (mode == QLatin1String("paranoia")) {
        ui->rbParanoiaExtract->setChecked(true);
    } else {
        ui->rbFastExtract->setChecked(true);
    }
    QString format = settings.value(QStringLiteral("Format"), QStringLiteral("flac")).toString();
    if (format == QLatin1String("flac")) {
        ui->rbFlac->setChecked(true);
    } else if (format == QLatin1String("mp3") && ui->rbMp3->isEnabled()) {
        ui->rbMp3->setChecked(true);
    } else if (format == QLatin1String("wav")) {
        ui->rbWav->setChecked(true);
    }
}

ExtractParametersDialog::~ExtractParametersDialog()
{
    delete ui;
}

void ExtractParametersDialog::setAlbumInfo(const QString &title, const QString &artist)
{
    m_album = title;
    m_artist = artist;
    ui->cbCreateSubfolder->setText(tr("Create Subfolder: %1/%2").arg(FileUtil::sanitizeFilename(artist)).arg(FileUtil::sanitizeFilename(title)));
}

QString ExtractParametersDialog::outputDir() const
{
    QString dir = ui->eOutputDirectory->text();
    if (ui->cbCreateSubfolder->isChecked())
        dir = QStringLiteral("%1/%2/%3").arg(dir, FileUtil::sanitizeFilename(m_artist), FileUtil::sanitizeFilename(m_album));

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

void ExtractParametersDialog::on_bStart_clicked()
{
    QSettings settings;
    settings.setValue(QStringLiteral("Output Directory"), ui->eOutputDirectory->text());
    settings.setValue(QStringLiteral("Create Subfolder"), ui->cbCreateSubfolder->isChecked());
    if (ui->rbParanoiaExtract->isChecked()) {
        settings.setValue(QStringLiteral("Paranoia Mode"), QStringLiteral("paranoia"));
    } else {
        settings.setValue(QStringLiteral("Paranoia Mode"), QStringLiteral("fast"));
    }
    settings.setValue(QStringLiteral("Format"), format());

    accept();
}


void ExtractParametersDialog::showEvent(QShowEvent *event)
{
    resize(width(), heightForWidth(width()));

    QDialog::showEvent(event);
}
