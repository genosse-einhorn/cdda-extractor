#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "libcdda/drive_handle.h"
#include "musicbrainz/releasefinder.h"
#include "tracklistmodel.h"
#include "uiutil/extendederrordialog.h"
#include "extractparametersdialog.h"
#include "uiutil/futureprogressdialog.h"
#include "musicbrainzaskdialog.h"
#include "extractor.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QMenu>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_progressDialog = new FutureProgressDialog(this);
    m_progressDialog->setModal(true);
    m_progressDialog->setCancelAllowed(true);

    // Generate extras menu
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Automatic Metadata Download Settings"), this, &MainWindow::changeMetadataSettingsClicked);
    menu->addAction(tr("Advanced Settings"), [=]() {
        SettingsDialog d(this);
        d.setModal(true);
        d.exec();
    });
    menu->addSeparator();
    menu->addAction(tr("About"));
    menu->addAction(tr("About Qt"), [=]() { QMessageBox::aboutQt(this); });


    ui->tbMore->setMenu(menu);

    connect(ui->tbRefresh, &QAbstractButton::clicked, this, &MainWindow::reloadToc);
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

    // set toolbar icons from theme
    QIcon themeRefresh = QIcon::fromTheme(QStringLiteral("view-refresh"));
    if (!themeRefresh.isNull())
        ui->tbRefresh->setIcon(themeRefresh);
    else
        ui->tbRefresh->setIcon(QIcon(QStringLiteral(":/view-refresh.svg")));

    QIcon themeSave = QIcon::fromTheme(QStringLiteral("document-save"));
    if (!themeSave.isNull())
        ui->tbExtract->setIcon(themeSave);
    else
        ui->tbExtract->setIcon(QIcon(QStringLiteral(":/document-save.svg")));

    QIcon themeHelp = QIcon::fromTheme(QStringLiteral("help-browser"));
    if (!themeHelp.isNull())
        ui->tbMore->setIcon(themeHelp);
    else
        ui->tbMore->setIcon(QIcon(QStringLiteral(":/help-browser.svg")));

    ui->iwStartLogo->setIcon(QIcon(QStringLiteral(":/logo.svg")));
    ui->iwNoCdLogo->setIcon(QIcon(QStringLiteral(":/cdcase.svg")));

    m_trackmodel = new TrackListModel(this);
    ui->tvTracks->setModel(m_trackmodel);

    connect(ui->eArtist, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumArtist);
    connect(ui->eComposer, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumComposer);
    connect(ui->eTitle, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumTitle);
    connect(ui->eGenre, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumGenre);
    connect(ui->eYear, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumYear);
    connect(ui->eDiscNo, &QLineEdit::textChanged, m_trackmodel, &TrackListModel::setAlbumDiscNo);

    connect(ui->tvTracks->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableHeaderClicked);

    resetUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reloadToc()
{
    resetUi();

    bool musicbrainzOk = MusicBrainzAskDialog::downloadOkMaybeAsk(this);

    m_progressDialog->setWindowTitle(tr("Loading Table of Contents..."));

    auto future = TaskRunner::run([=](const TaskRunner::CancelToken &cancelToken, const TaskRunner::ProgressToken &progressToken) {
        progressToken.reportProgressValueAndText(1, tr("Loading TOC from CD..."));

        QStringList errorLog;
        QString device;
        enum cdda::result_sense errorSense = cdda::RESULT_SENSE_OK;
        cdda::toc toc = cdda::find_toc(&device, &errorLog, &errorSense, cancelToken);

        MusicBrainz::ReleaseMetadata release;

        if (toc.is_valid() && !cancelToken.isCanceled() && musicbrainzOk) {
            progressToken.reportProgressValueAndText(2, tr("Searching for metadata..."));
            release = MusicBrainz::findRelease(cdda::calculate_musicbrainz_discid(toc),
                                               toc.catalog,
                                               cancelToken);
        }

        return std::make_tuple(toc, device, errorLog, errorSense, release);
    });

    m_progressDialog->setFuture(future);

    TaskRunner::handle_result_tuple_unpack(future, this, &MainWindow::tocLoaded);
}

void MainWindow::tocLoaded(const cdda::toc &toc, const QString &device, const QStringList &errorLog, cdda::result_sense errorSense, const MusicBrainz::ReleaseMetadata &release)
{
    if (toc.is_valid())
    {
        ui->stackedWidget->setCurrentWidget(ui->metadataPage);
        ui->tbExtract->setEnabled(true);

        ui->eArtist->setText(toc.artist);
        ui->eTitle->setText(toc.title);

        m_trackmodel->reset(toc.tracks);
        m_trackmodel->setDevice(device);

        if (release.artist.size())
            ui->eArtist->setText(release.artist);

        if (release.title.size())
            ui->eTitle->setText(release.title);

        if (release.composer.size())
            ui->eComposer->setText(release.composer);

        if (release.year.size())
            ui->eYear->setText(release.year);

        if (release.discNo.size())
            ui->eDiscNo->setText(release.discNo);

        if (!release.cover.isNull())
        {
            ui->coverArt->setCover(release.cover);
            m_trackmodel->setAlbumCover(release.cover);
        }

        for (const auto &track : release.tracks)
        {
            int i = m_trackmodel->trackIndexForTrackno(track.trackno);
            if (i < 0)
                continue;

            if (track.artist.size())
                m_trackmodel->setTrackArtist(i, track.artist);

            if (track.title.size())
                m_trackmodel->setTrackTitle(i, track.title);

            if (track.composer.size())
                m_trackmodel->setTrackComposer(i, track.composer);
        }
    }
    else
    {
        if (errorSense == cdda::RESULT_SENSE_NOMEDIUM) {
            ui->stackedWidget->setCurrentWidget(ui->nocdPage);
        } else {
            ExtendedErrorDialog::show(this, tr("Failed to load the table of contents from the CD"),
                                       errorLog.join(QStringLiteral("\n")));
        }
    }
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

    ui->tbExtract->setEnabled(false);

    ui->stackedWidget->setCurrentWidget(ui->startPage);
}

void MainWindow::beginExtract()
{
    ExtractParametersDialog dialog(this);
    dialog.setAlbumInfo(m_trackmodel->albumTitle(), m_trackmodel->albumArtist());
    if (!dialog.exec())
        return;

    std::vector<Extractor::TrackToExtract> tracks;
    for (int i = 0; i < m_trackmodel->trackCount(); ++i) {
        if (m_trackmodel->trackSelected(i)) {
            Extractor::TrackToExtract t;
            t.start = m_trackmodel->trackBegin(i);
            t.length = m_trackmodel->trackLength(i);
            t.metadata = m_trackmodel->trackMetadata(i);
            tracks.push_back(t);
        }
    }

    QFuture<QString> f = Extractor::extract(m_trackmodel->device(),
                                            dialog.outputDir(),
                                            dialog.format(),
                                            tracks,
                                            dialog.paranoiaActivated() ? Extractor::READ_PARANOIA : Extractor::READ_FAST);

    m_progressDialog->setWindowTitle(tr("Extracting Audio..."));
    m_progressDialog->setFuture(f);


    TaskRunner::handle_result(f, this, [=](const QString &error){
        if (error.size()) {
            ExtendedErrorDialog::show(this, tr("Audio extraction failed."), error);
        } else {
            QMessageBox::information(this, tr("Success"), tr("Audio extraction completed successfully."), QMessageBox::Ok);
        }
    });
}

void MainWindow::tableHeaderClicked(int logicalIndex)
{
    if (logicalIndex == TrackListModel::COLUMN_TRACKNO) {
        // Checkmark column - check/uncheck all

        bool allChecked = true;
        for (int i = 0; i < m_trackmodel->trackCount(); ++i) {
            if (!m_trackmodel->trackSelected(i)) {
                allChecked = false;
                break;
            }
        }

        for (int i = 0; i < m_trackmodel->trackCount(); ++i) {
            m_trackmodel->setData(m_trackmodel->index(i, TrackListModel::COLUMN_TRACKNO), allChecked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
        }
    }
}

void MainWindow::changeMetadataSettingsClicked()
{
    if (MusicBrainzAskDialog::showAskDialog(this)) {
        if (QMessageBox::question(this, tr("Refresh Required"),
                                  tr("Do you want to download metadata now? Any metadata you may have already entered manually will be erased."),
                                  tr("No"), tr("Refresh metadata now"), QString(), 1, 0))
            this->reloadToc();
    }
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

    QTimer::singleShot(1, this, &MainWindow::reloadToc);
}
