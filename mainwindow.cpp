#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "libcdda/drive_handle.h"
#include "musicbrainz/releasefinder.h"
#include "tracklistmodel.h"
#include "uiutil/extendederrordialog.h"
#include "extractparametersdialog.h"
#include "extractrunner.h"
#include "uiutil/progressdialog.h"

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

    // set toolbar icons from standard pixmaps
    QIcon themeRefresh = QIcon::fromTheme(QStringLiteral("view-refresh"));
    QIcon standardRefresh = this->style()->standardIcon(QStyle::SP_BrowserReload);
    if (!themeRefresh.isNull())
        ui->tbRefresh->setIcon(themeRefresh);
    else if (!standardRefresh.isNull())
        ui->tbRefresh->setIcon(standardRefresh);
    else
        ui->tbRefresh->setIcon(QIcon(QStringLiteral(":/view-refresh.svg")));

    QIcon themeSave = QIcon::fromTheme(QStringLiteral("document-save"));
    QIcon standardSave = this->style()->standardIcon(QStyle::SP_DialogSaveButton);
    if (!themeSave.isNull())
        ui->tbExtract->setIcon(themeSave);
    else if (!standardSave.isNull())
        ui->tbExtract->setIcon(standardSave);
    else
        ui->tbRefresh->setIcon(QIcon(QStringLiteral(":/document-save.svg")));

    QIcon themeHelp = QIcon::fromTheme(QStringLiteral("help-browser"));
    QIcon standardHelp = this->style()->standardIcon(QStyle::SP_DialogHelpButton);
    if (!themeHelp.isNull())
        ui->tbMore->setIcon(themeHelp);
    else if (!standardHelp.isNull())
        ui->tbMore->setIcon(standardHelp);
    else
        ui->tbMore->setIcon(QIcon(QStringLiteral(":/help-browser.svg")));

    m_trackmodel = new TrackListModel(this);
    ui->tvTracks->setModel(m_trackmodel);

    connect(&m_tocFindFutureWatcher, &QFutureWatcherBase::finished, this, &MainWindow::tocLoadFinish);
    connect(&m_musicbrainzFutureWatcher, &QFutureWatcherBase::finished, this, &MainWindow::musicbrainzReleaseFound);

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
        m_tocReadProgressDialog->setCancelAllowed(true);
        connect(m_tocReadProgressDialog, &ProgressDialog::canceled, &m_tocFindFutureWatcher, &QFutureWatcherBase::cancel);
    }

    m_tocReadProgressDialog->setLabelText(tr("Loading Table of Contents from CD..."));
    m_tocReadProgressDialog->show();

    m_tocFindFutureWatcher.setFuture(cdda::find_toc());
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

void MainWindow::tocLoadFinish()
{
    if (m_tocFindFutureWatcher.isCanceled())
    {
        delete m_tocReadProgressDialog;
        m_tocReadProgressDialog = nullptr;

        return;
    }

    cdda::toc_find_result result = m_tocFindFutureWatcher.result();
    if (result.toc.is_valid())
    {
        ui->eArtist->setText(result.toc.artist);
        ui->eTitle->setText(result.toc.title);

        m_trackmodel->reset(result.toc.tracks);
        m_trackmodel->setDevice(result.device);
        ui->tvTracks->setEnabled(true);
        ui->metadataWidget->setEnabled(true);
        ui->tbExtract->setEnabled(true);

        m_tocReadProgressDialog->setLabelText(tr("Searching for metadata..."));
        connect(m_tocReadProgressDialog, &ProgressDialog::canceled, &m_musicbrainzFutureWatcher, &QFutureWatcherBase::cancel);

        m_musicbrainzFutureWatcher.setFuture(MusicBrainz::findReleaseOnThread(
                                                 cdda::calculate_musicbrainz_discid(result.toc),
                                                 result.toc.catalog));
    }
    else
    {
        delete m_tocReadProgressDialog;
        m_tocReadProgressDialog = nullptr;

        ExtendedErrorDialog::show(this, tr("Failed to load the table of contents from the CD.\n\n"
                                           "Make sure an audio CD is inserted into the drive."),
                                  result.log.join(QStringLiteral("\n")));
    }
}

void MainWindow::musicbrainzReleaseFound()
{
    if (m_musicbrainzFutureWatcher.isCanceled()) {
        delete m_tocReadProgressDialog;
        m_tocReadProgressDialog = nullptr;
        return;
    }

    MusicBrainz::ReleaseMetadata release = m_musicbrainzFutureWatcher.result();
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

    delete m_tocReadProgressDialog;
    m_tocReadProgressDialog = nullptr;
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
