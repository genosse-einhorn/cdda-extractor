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

    void tocLoadFinish();
    void musicbrainzReleaseFound();

    void extractError(const QString &msg);
    void extractSuccess();

private:
    Ui::MainWindow *ui;

    TrackListModel *m_trackmodel { nullptr };

    QFutureWatcher<cdda::toc_find_result> m_tocFindFutureWatcher;
    QFutureWatcher<MusicBrainz::ReleaseMetadata> m_musicbrainzFutureWatcher;

    ProgressDialog *m_tocReadProgressDialog { nullptr };

    bool m_initialLoadDone { false };

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // MAINWINDOW_H
