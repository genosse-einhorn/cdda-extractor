#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <QCoreApplication>
#include <QFuture>
#include <vector>
#include "libcdda/toc.h"

class Extractor
{
    Q_DECLARE_TR_FUNCTIONS(Extractor)

    Extractor() = delete;

public:
    struct TrackToExtract {
        cdda::block_addr start;
        cdda::block_addr_delta length;
        cdda::track_metadata metadata;
    };

    enum ReadingMode {
        READ_FAST,
        READ_PARANOIA
    };

    QFuture<QString> static extract(const QString &device, const QString &outdir, const QString &format,
                                    const std::vector<TrackToExtract> &tracks, ReadingMode mode);
};

#endif // EXTRACTOR_H
