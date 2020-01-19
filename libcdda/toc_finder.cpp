#include "toc_finder.h"

#include "drive_handle.h"
#include "../tasklib/taskrunner.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace cdda {

toc find_toc(QString *out_device, QStringList *out_log, const TaskRunner::CancelToken &cancelToken)
{
    QStringList drives = cdda::drive_handle::list_drives();

    if (!drives.size())
        *out_log << toc_finder::tr("No CD drives found :(");

    for (QString drive : drives)
    {
        if (cancelToken.isCanceled())
            return cdda::toc();

        *out_log << toc_finder::tr("Probing drive: %1").arg(drive);

        cdda::drive_handle h = cdda::drive_handle::open(drive);
        if (!h.ok())
        {
            *out_log << toc_finder::tr("Could not open %1: %2").arg(drive).arg(h.last_error());
            continue;
        }

        if (cancelToken.isCanceled())
            return cdda::toc();

        auto toc = h.get_toc();
        if (!toc.is_valid())
        {
            *out_log << toc_finder::tr("Could not read TOC from %1: %2").arg(drive).arg(h.last_error());
            continue;
        }

        auto audio = std::find_if(toc.tracks.cbegin(), toc.tracks.cend(), [](const toc_track &track) { return track.is_audio(); });
        if (audio == toc.tracks.cend())
        {
            *out_log << toc_finder::tr("CD in %1 does not contain any audio tracks.").arg(drive);
            continue;
        }

        if (cancelToken.isCanceled())
            return toc;

        //=== find media catalog number
        h.fill_mcn(toc);

        //=== find ISRC for tracks
        for (toc_track &track : toc.tracks)
        {
            if (cancelToken.isCanceled())
                return toc;

            h.fill_track_isrc(track);
        }

        if (cancelToken.isCanceled())
            return toc;

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

        *out_device = h.device_name();
        return toc;
    }

    return cdda::toc();
}

QFuture<toc_find_result> find_toc_threaded()
{
    return TaskRunner::run([](const TaskRunner::CancelToken &cancelToken, const TaskRunner::ProgressToken&){
        cdda::toc_find_result result;

        result.toc = find_toc(&result.device, &result.log, cancelToken);

        return result;
    });
}


} // namespace cdda
