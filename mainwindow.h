#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "libcdda/toc_finder.h"
#include "musicbrainz/releasefinder.h"

namespace Ui {
class MainWindow;
}

class TrackListModel;
class ProgressDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void showToc();
    void resetUi();
    void beginExtract();

    void tocLoadSuccess(const QString &device, const cdda::toc &tracks);
    void tocLoadError(const QString &msg);
    void tocLoadFinish();
    void musicbrainzReleaseFound(const MusicBrainz::ReleaseMetadata &release);

    void extractError(const QString &msg);
    void extractSuccess();

private:
    Ui::MainWindow *ui;

    TrackListModel *m_trackmodel { nullptr };

    cdda::toc_finder m_tocReader;
    MusicBrainz::ReleaseFinder m_releaseFinder;
    ProgressDialog *m_tocReadProgressDialog { nullptr };

    bool m_initialLoadDone { false };

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // MAINWINDOW_H
