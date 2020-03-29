#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QFutureWatcher>

#include "libcdda/toc_finder.h"
#include "musicbrainz/releasefinder.h"

namespace Ui {
class MainWindow;
}

class TrackListModel;
class ProgressDialog;
class FutureProgressDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void reloadToc();
    void tocLoaded(const cdda::toc &toc, const QString &device, const QStringList &errorLog, enum cdda::result_sense errorSense, const MusicBrainz::ReleaseMetadata &release);

    void resetUi();
    void beginExtract();

    void tableHeaderClicked(int logicalIndex);
    void changeMetadataSettingsClicked();

    void showAboutDialog();

private:
    Ui::MainWindow *ui;

    TrackListModel *m_trackmodel { nullptr };

    FutureProgressDialog *m_progressDialog;

    bool m_initialLoadDone { false };

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // MAINWINDOW_H
