#include "extractparametersdialog.h"
#include "ui_extractparametersdialog.h"
#include "encoder/lame_backend.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>

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
