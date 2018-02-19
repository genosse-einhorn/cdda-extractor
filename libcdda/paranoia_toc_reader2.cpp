#include "paranoia_toc_reader2.h"

#include "toc.h"

#include <QtConcurrent>

#include <paranoia/cdda.h>
#include <paranoia/paranoia.h>
#include <cstring>
#include <cstdlib>

namespace {

struct toc_result
{
    QStringList errors;
    std::vector<cdda::toc_track> tracks;
};

toc_result readToc()
{
    toc_result retval;

    char *messages = nullptr;
    cdrom_drive_t *drive = cdda_find_a_cdrom(CDDA_MESSAGE_LOGIT, &messages);
    track_t numTracks = 0;
    int err = 0;

    if (messages)
    {
        retval.errors << QString::fromUtf8(messages);
        std::free(messages);
        messages = nullptr;
    }

    if (!drive)
        goto out;

    err = cdda_open(drive);
    if (err != 0)
    {
        messages = cdda_messages(drive);
        if (messages)
        {
            retval.errors << QString::fromUtf8(messages);
            std::free(messages);
            messages = nullptr;
        }

        messages = cdda_errors(drive);
        if (messages)
        {
            retval.errors << QString::fromUtf8(messages);
            std::free(messages);
            messages = nullptr;
        }
        goto out;
    }

    numTracks = cdda_tracks(drive);
    for (track_t i = 0; i < numTracks; ++i)
    {
        if (!cdda_track_audiop(drive, i))
            continue;

        cdda::toc_track track;
        track.index = int(i);
        track.start = cdda::block_addr::from_lba(int(cdda_track_firstsector(drive, i)));
        track.length = cdda::block_addr::from_lba(int(cdda_track_lastsector(drive, i))+1) - track.start;
        retval.tracks.push_back(track);
    }

out:
    if (drive) cdda_close(drive);
    return retval;
}

} // anonymous namespace

namespace cdda {

paranoia_toc_reader2::paranoia_toc_reader2(QObject *parent) : QObject(parent)
{

}

void paranoia_toc_reader2::start()
{
    auto future = QtConcurrent::run(readToc);
    QFutureWatcher<toc_result> *w  = new QFutureWatcher<toc_result>(this);
    connect(w, &QFutureWatcherBase::finished, this, [=]() {
        auto res = w->result();
        if (res.tracks.size())
        {
            emit success(res.tracks);
        }
        else
        {
            emit error(res.errors.join(QStringLiteral("\n")));
        }
    });
    connect(w, &QFutureWatcherBase::finished, w, &QObject::deleteLater);
    w->setFuture(future);
}

} // namespace cdda
