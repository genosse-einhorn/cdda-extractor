#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "libcdda/drive_handle.h"
#include "musicbrainz/releasefinder.h"
#include "tracklistmodel.h"
#include "extendederrordialog.h"
#include "extractparametersdialog.h"
#include "extractrunner.h"
#include "progressdialog.h"

#include <QMessageBox>
#include <QMenu>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Generate extras menu
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("About"));
    menu->addAction(tr("About Qt"), [=]() { QMessageBox::aboutQt(this); });

    ui->tbMore->setMenu(menu);

    connect(ui->tbRefresh, &QAbstractButton::clicked, this, &MainWindow::showToc);
    connect(ui->tbExtract, &QAbstractButton::clicked, this, &MainWindow::beginExtract);

    // Enlarge "Extract" button
    auto font = ui->tbExtract->font();
    font.setPointSizeF(font.pointSizeF()*1.2);
    ui->tbExtract->setFont(font);

    // convert toolbar layout to real toolbar
    QLayoutItem *child;
    while ((child = ui->hlToolbar->takeAt(0)))
    {
        auto frame = qobject_cast<QFrame*>(child->widget());
        auto toolbutton = qobject_cast<QToolButton*>(child->widget());

        // remove small menu indicator arrow
        if (toolbutton)
            toolbutton->setStyleSheet(QStringLiteral("QToolButton::menu-indicator { image: none; }"));

        if (frame && frame->frameShape() == QFrame::VLine)
        {
            ui->toolBar->addSeparator();
            delete frame;
        }
        else
        {
            ui->toolBar->addWidget(child->widget());
        }

        delete child;
    }
    delete ui->hlToolbar;
    ui->hlToolbar = nullptr;

    m_trackmodel = new TrackListModel(this);
    ui->tvTracks->setModel(m_trackmodel);

    connect(&m_tocReader, &cdda::toc_finder::success, this, &MainWindow::tocLoadSuccess);
    connect(&m_tocReader, &cdda::toc_finder::error, this, &MainWindow::tocLoadError);
    connect(&m_releaseFinder, &MusicBrainz::ReleaseFinder::noMetadataFound, this, &MainWindow::tocLoadFinish);
    connect(&m_releaseFinder, &MusicBrainz::ReleaseFinder::metadataFound, this, &MainWindow::musicbrainzReleaseFound);

    connect(ui->eArtist, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumArtist);
    connect(ui->eComposer, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumComposer);
    connect(ui->eTitle, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumTitle);
    connect(ui->eGenre, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumGenre);
    connect(ui->eYear, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumYear);
    connect(ui->eDiscNo, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumDiscNo);

    resetUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showToc()
{
    resetUi();

    if (!m_tocReadProgressDialog)
    {
        m_tocReadProgressDialog = new ProgressDialog(this);
        m_tocReadProgressDialog->setModal(true);
        m_tocReadProgressDialog->setWindowTitle(tr("Loading Table of Contents..."));
        m_tocReadProgressDialog->setCancelAllowed(false);
    }

    m_tocReadProgressDialog->setLabelText(tr("Loading Table of Contents from CD..."));
    m_tocReadProgressDialog->show();
    m_tocReader.start();
}

void MainWindow::resetUi()
{
    m_trackmodel->clear();

    ui->eTitle->clear();
    ui->eArtist->clear();
    ui->eComposer->clear();
    ui->eGenre->clear();
    ui->eYear->clear();
    ui->eDiscNo->clear();
    ui->coverArt->resetCover();

    ui->tvTracks->setEnabled(false);
    ui->metadataWidget->setEnabled(false);
    ui->tbExtract->setEnabled(false);

}

void MainWindow::beginExtract()
{
    ExtractParametersDialog dialog(this);
    dialog.setAlbumInfo(m_trackmodel->albumTitle(), m_trackmodel->albumArtist());
    if (!dialog.exec())
        return;

    ProgressDialog *pdialog = new ProgressDialog(this);
    ExtractRunner *runner = new ExtractRunner(m_trackmodel, this);
    connect(runner, &ExtractRunner::finished, runner, &QObject::deleteLater);
    connect(runner, &ExtractRunner::finished, pdialog, &QObject::deleteLater);
    connect(runner, &ExtractRunner::finished, this, &MainWindow::extractSuccess);
    connect(runner, &ExtractRunner::failed, runner, &QObject::deleteLater);
    connect(runner, &ExtractRunner::failed, pdialog, &QObject::deleteLater);
    connect(runner, &ExtractRunner::failed, this, &MainWindow::extractError);
    connect(runner, &ExtractRunner::progress, pdialog, &ProgressDialog::setValue);
    connect(runner, &ExtractRunner::status, pdialog, &ProgressDialog::setLabelText);
    connect(pdialog, &ProgressDialog::canceled, runner, &ExtractRunner::cancel);

    pdialog->setWindowTitle(tr("Extracting audio..."));
    pdialog->setMinimum(runner->progressMin());
    pdialog->setMaximum(runner->progressMax());
    pdialog->setLabelText(tr("Initializing..."));
    pdialog->setModal(true);
    pdialog->show();

    runner->setOutputDirectory(dialog.outputDir());
    runner->setFormat(dialog.format());
    runner->setUseParanoia(dialog.paranoiaActivated());

    runner->start();
}

void MainWindow::tocLoadSuccess(const QString &device, const cdda::toc &toc)
{
    ui->eArtist->setText(toc.artist);
    ui->eTitle->setText(toc.title);

    m_trackmodel->reset(toc.tracks);
    m_trackmodel->setDevice(device);
    ui->tvTracks->setEnabled(true);
    ui->metadataWidget->setEnabled(true);
    ui->tbExtract->setEnabled(true);

    m_tocReadProgressDialog->setLabelText(tr("Searching for metadata..."));
    m_releaseFinder.startSearch(cdda::calculate_musicbrainz_discid(toc), toc.catalog);
}

void MainWindow::tocLoadError(const QString &msg)
{
    tocLoadFinish();
    ExtendedErrorDialog::show(this, tr("Failed to load the table of contents from the CD.\n\n"
                                       "Make sure an audio CD is inserted into the drive."), msg);
}

void MainWindow::tocLoadFinish()
{
    delete m_tocReadProgressDialog;
    m_tocReadProgressDialog = nullptr;
}

void MainWindow::musicbrainzReleaseFound(const MusicBrainz::ReleaseMetadata &release)
{
    ui->eArtist->setText(release.artist);
    ui->eTitle->setText(release.title);
    ui->eComposer->setText(release.composer);
    ui->eYear->setText(release.year);
    ui->eDiscNo->setText(release.discNo);
    ui->coverArt->setCover(release.cover);
    m_trackmodel->setAlbumCover(release.cover);
    for (const auto &track : release.tracks)
    {
        int i = m_trackmodel->trackIndexForTrackno(track.trackno);
        if (i < 0)
            continue;

        m_trackmodel->setTrackArtist(i, track.artist);
        m_trackmodel->setTrackTitle(i, track.title);
        m_trackmodel->setTrackComposer(i, track.composer);
    }

    tocLoadFinish();
}

void MainWindow::extractError(const QString &msg)
{
    ExtendedErrorDialog::show(this, tr("Audio extraction failed."), msg);
}

void MainWindow::extractSuccess()
{
    QMessageBox::information(this, tr("Success"), tr("Audio extraction completed successfully."), QMessageBox::Ok);
}


void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (m_initialLoadDone)
        return;

    m_initialLoadDone = true;

    int metadatawidth = ui->tvTracks->width()
            - ui->tvTracks->columnWidth(TrackListModel::COLUMN_TRACKNO)
            - ui->tvTracks->columnWidth(TrackListModel::COLUMN_LENGTH);
    ui->tvTracks->setColumnWidth(TrackListModel::COLUMN_TITLE, metadatawidth/3);
    ui->tvTracks->setColumnWidth(TrackListModel::COLUMN_ARTIST, metadatawidth/3);
    ui->tvTracks->setColumnWidth(TrackListModel::COLUMN_COMPOSER, metadatawidth/3);
    ui->tvTracks->horizontalHeader()->setSectionResizeMode(TrackListModel::COLUMN_TRACKNO, QHeaderView::ResizeToContents);
    ui->tvTracks->horizontalHeader()->setSectionResizeMode(TrackListModel::COLUMN_LENGTH, QHeaderView::ResizeToContents);

    QTimer::singleShot(1, this, &MainWindow::showToc);
}
