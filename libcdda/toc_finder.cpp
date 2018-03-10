#include "toc_finder.h"

#include "drive_handle.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace cdda {

toc_finder::toc_finder(QObject *parent) : QObject(parent)
{

}

void toc_finder::start()
{
    auto future = QtConcurrent::run(toc_finder::readToc);
    QFutureWatcher<toc_result> *w  = new QFutureWatcher<toc_result>(this);
    connect(w, &QFutureWatcherBase::finished, this, [=]() {
        auto res = w->result();
        if (res.toc.is_valid())
        {
            emit success(res.device, res.toc);
        }
        else
        {
            emit error(res.log.join(QStringLiteral("\n")));
        }
    });
    connect(w, &QFutureWatcherBase::finished, w, &QObject::deleteLater);
    w->setFuture(future);
}

toc_finder::toc_result toc_finder::readToc()
{
    toc_result retval;

    QStringList drives = cdda::drive_handle::list_drives();

    if (!drives.size())
        retval.log << tr("No CD drives found :(");

    for (QString drive : drives)
    {
        retval.log << tr("Probing drive: %1").arg(drive);

        cdda::drive_handle h = cdda::drive_handle::open(drive);
        if (!h.ok())
        {
            retval.log << tr("Could not open %1: %2").arg(drive).arg(h.last_error());
            continue;
        }

        auto toc = h.get_toc();
        if (!toc.is_valid())
        {
            retval.log << tr("Could not read TOC from %1: %2").arg(drive).arg(h.last_error());
            continue;
        }

        auto audio = std::find_if(toc.tracks.cbegin(), toc.tracks.cend(), [](const toc_track &track) { return track.is_audio(); });
        if (audio == toc.tracks.cend())
        {
            retval.log << tr("CD in %1 does not contain any audio tracks.").arg(drive);
            continue;
        }

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

} // namespace cdda
