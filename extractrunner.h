#ifndef EXTRACTRUNNER_H
#define EXTRACTRUNNER_H

#include <QObject>

#include "libcdda/toc.h"

class QThread;

class TrackListModel;
class ExtractRunnerWorker;

class ExtractRunner : public QObject
{
    Q_OBJECT
public:
    explicit ExtractRunner(const TrackListModel *model, QObject *parent = nullptr);
    ~ExtractRunner();

    int progressMin() const;
    int progressMax() const;
    int progressCurrent() const { return m_processedBlocks.delta_blocks; }

signals:
    void finished();
    void failed(const QString &errorMessages);
    void progress(int current);
    void status(const QString &messages);

public slots:
    void setFormat(const QString &format);
    void setOutputDirectory(const QString &dir);
    void setUseParanoia(bool paranoia);

    void start();
    void cancel();

private slots:
    void workerProgress(cdda::block_addr_delta blocks);
    void sendNextTrackToWorker();

private:
    const TrackListModel *m_trackList;
    QString m_format { QStringLiteral("wav") };
    QString m_outputDirectory;
    bool m_paranoia { false };

    QThread *m_workerThread;
    ExtractRunnerWorker *m_worker;

    int m_currentTrackIndex;
    cdda::block_addr_delta m_processedBlocksInCurrentTrack;
    cdda::block_addr_delta m_processedBlocks;
};

#endif // EXTRACTRUNNER_H
