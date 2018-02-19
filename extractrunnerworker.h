#ifndef EXTRACTRUNNERWORKER_H
#define EXTRACTRUNNERWORKER_H

#include <QObject>
#include "libcdda/toc.h"
#include "libcdda/drive_handle.h"

class ExtractRunnerWorker : public QObject
{
    Q_OBJECT
public:
    explicit ExtractRunnerWorker(QObject *parent = nullptr);

signals:
    void finished();
    void failed(const QString &message);
    void progress(cdda::block_addr_delta numFrames);

public slots:
    void openDevice(const QString &device);
    void beginExtract(const QString &directory, const QString &basename, const QString &format,
                      cdda::block_addr start, cdda::block_addr_delta length, const cdda::track_metadata &metadata);
    void cancel();

private:
    cdda::drive_handle m_handle;
    bool m_cancelRequested { false };
};

#endif // EXTRACTRUNNERWORKER_H
