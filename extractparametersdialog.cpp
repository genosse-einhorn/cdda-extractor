#include "extractparametersdialog.h"
#include "ui_extractparametersdialog.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>

ExtractParametersDialog::ExtractParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtractParametersDialog)
{
    ui->setupUi(this);

    ui->eOutputDirectory->setText(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
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

void ExtractParametersDialog::on_bBrowseDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select output directory"),
                                                    ui->eOutputDirectory->text());
    if (dir.size())
        ui->eOutputDirectory->setText(dir);
}