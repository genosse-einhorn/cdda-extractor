#ifndef MUSICBRAINZ_RELEASEFINDER2_H
#define MUSICBRAINZ_RELEASEFINDER2_H

#include "../tasklib/taskrunner.h"

#include <QImage>
#include <vector>

namespace MusicBrainz {

struct TrackMetadata {
    int trackno;
    QString title;
    QString artist;
    QString composer;
};

struct ReleaseMetadata {
    QString releaseId;
    QString title;
    QString artist;
    QString composer;
    QString year;
    QString discNo;
    QImage cover;
    std::vector<TrackMetadata> tracks;
};

ReleaseMetadata findRelease(const QString &discid, const QString &mcn, const TaskRunner::CancelToken &cancelToken);

QFuture<ReleaseMetadata> findReleaseOnThread(const QString &discid, const QString &mcn);

} // namespace MusicBrainz

#endif // MUSICBRAINZ_RELEASEFINDER2_H
