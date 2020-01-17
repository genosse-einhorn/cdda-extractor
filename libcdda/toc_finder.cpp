#include "toc_finder.h"

#include "drive_handle.h"
#include "../tasklib/taskrunner.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace cdda {

static toc_find_result intern_find_toc(const TaskRunner::CancelToken &cancelToken)
{
    toc_find_result retval;

    QStringList drives = cdda::drive_handle::list_drives();

    if (!drives.size())
        retval.log << toc_finder::tr("No CD drives found :(");

    for (QString drive : drives)
    {
        if (cancelToken.isCanceled())
            return retval;

        retval.log << toc_finder::tr("Probing drive: %1").arg(drive);

        cdda::drive_handle h = cdda::drive_handle::open(drive);
        if (!h.ok())
        {
            retval.log << toc_finder::tr("Could not open %1: %2").arg(drive).arg(h.last_error());
            continue;
        }

        if (cancelToken.isCanceled())
            return retval;

        auto toc = h.get_toc();
        if (!toc.is_valid())
        {
            retval.log << toc_finder::tr("Could not read TOC from %1: %2").arg(drive).arg(h.last_error());
            continue;
        }

        auto audio = std::find_if(toc.tracks.cbegin(), toc.tracks.cend(), [](const toc_track &track) { return track.is_audio(); });
        if (audio == toc.tracks.cend())
        {
            retval.log << toc_finder::tr("CD in %1 does not contain any audio tracks.").arg(drive);
            continue;
        }

        if (cancelToken.isCanceled())
            return retval;

        //=== find media catalog number
        h.fill_mcn(toc);

        //=== find ISRC for tracks
        for (toc_track &track : toc.tracks)
        {
            if (cancelToken.isCanceled())
                return retval;

            h.fill_track_isrc(track);
        }

        if (cancelToken.isCanceled())
            return retval;

        //=== search CD-TEXT
        h.fill_cd_text(toc);

        // if the first track starts after 00:02.00, something fishy is going on
        if (toc.tracks[0].start > cdda::block_addr::from_lba(0))
        {
            // synthesize track zero
            cdda::toc_track track;
            track.session = toc.tracks[0].session;
            track.adr = toc.tracks[0].adr;
            track.control = toc.tracks[0].control;
            track.index = 0;
            track.start = cdda::block_addr::from_lba(0);
            track.length = toc.tracks[0].start - track.start;

            toc.tracks.insert(toc.tracks.begin(), track);
        }

        retval.device = h.device_name();
        retval.toc = toc;
        break;
    }

    return retval;
}

QFuture<toc_find_result> find_toc()
{
    return TaskRunner::run(intern_find_toc);
}

} // namespace cdda
