#include "extractrunner.h"

#include <QThread>

#include "extractrunnerworker.h"
#include "tracklistmodel.h"

ExtractRunner::ExtractRunner(const TrackListModel *model, QObject *parent) : QObject(parent)
{
    m_trackList = model;

    m_workerThread = new QThread(this);
    m_worker = new ExtractRunnerWorker();
    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

    connect(m_worker, &ExtractRunnerWorker::failed, this, &ExtractRunner::failed);
    connect(m_worker, &ExtractRunnerWorker::finished, this, &ExtractRunner::sendNextTrackToWorker);
    connect(m_worker, &ExtractRunnerWorker::progress, this, &ExtractRunner::workerProgress);
}

ExtractRunner::~ExtractRunner()
{
    m_workerThread->quit();
    m_workerThread->wait();
}

int ExtractRunner::progressMin() const
{
    return 0;
}

int ExtractRunner::progressMax() const
{
    int m = 0;
    for (int i = 0; i < m_trackList->trackCount(); ++i)
    {
        if (m_trackList->trackSelected(i))
            m += m_trackList->trackLength(i).delta_blocks;
    }

    return m;
}

void ExtractRunner::setFormat(const QString &format)
{
    m_format = format;
}

void ExtractRunner::setOutputDirectory(const QString &dir)
{
    m_outputDirectory = dir;
}

void ExtractRunner::start()
{
    m_currentTrackIndex = -1;
    m_processedBlocks = cdda::block_addr_delta::from_lba(0);

    // open device in worker
    QMetaObject::invokeMethod(m_worker, "openDevice", Q_ARG(QString, m_trackList->device()));

    sendNextTrackToWorker();
}

void ExtractRunner::cancel()
{
    QMetaObject::invokeMethod(m_worker, "cancel");
}

void ExtractRunner::workerProgress(cdda::block_addr_delta blocks)
{
    m_processedBlocks += blocks;
    m_processedBlocksInCurrentTrack += blocks;
    emit progress(m_processedBlocks.delta_blocks);

    QString s = tr("Extracting Track %1 (%2)")
            .arg(m_trackList->trackNo(m_currentTrackIndex))
            .arg(m_processedBlocksInCurrentTrack.to_display());
    emit status(s);
}

void ExtractRunner::sendNextTrackToWorker()
{
    // find next track to work on
    m_currentTrackIndex++;
    m_processedBlocksInCurrentTrack = cdda::block_addr_delta::from_lba(0);

    while (m_currentTrackIndex < m_trackList->trackCount() && !m_trackList->trackSelected(m_currentTrackIndex))
        m_currentTrackIndex++;

    // check if we might be finished already
    if (m_currentTrackIndex >= m_trackList->trackCount())
    {
        emit finished();
        return;
    }

    // send metadata to worker

    // send track to worker
    QString basename = QStringLiteral("%1 - %2")
            .arg(m_trackList->trackNo(m_currentTrackIndex), 2, 10, QLatin1Char('0'))
            .arg(m_trackList->trackTitle(m_currentTrackIndex));
    QMetaObject::invokeMethod(m_worker, "beginExtract",
                              Q_ARG(QString, m_outputDirectory),
                              Q_ARG(QString, basename),
                              Q_ARG(QString, m_format),
                              Q_ARG(cdda::block_addr, m_trackList->trackBegin(m_currentTrackIndex)),
                              Q_ARG(cdda::block_addr_delta, m_trackList->trackLength(m_currentTrackIndex)),
                              Q_ARG(cdda::track_metadata, m_trackList->trackMetadata(m_currentTrackIndex)));
}
