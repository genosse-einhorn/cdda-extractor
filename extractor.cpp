// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "extractor.h"

#include "tasklib/taskrunner.h"
#include "libcdda/drive_handle.h"
#include "paranoia/paranoia.h"
#include "encoder/flacencoder.h"
#include "encoder/lameencoder.h"
#include "encoder/wavencoder.h"
#include "fileutil.h"
#include "ringbuffer.h"
#include "encoderwriter.h"

#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <QDebug>

namespace {

struct ReaderBase
{
    const TaskRunner::CancelToken &m_cancelToken;

    ReaderBase(const TaskRunner::CancelToken &cancelToken) : m_cancelToken(cancelToken) {}
    virtual ~ReaderBase() {}
    virtual bool eof() = 0;
    virtual bool read(qint16 **pBuffer, cdda::block_addr_delta *pBuflen) = 0;
    virtual QString errorMessage() = 0;
};

class RawReader : public ReaderBase
{
private:
    cdda::drive_handle &m_handle;
    cdda::block_addr m_pos;
    cdda::block_addr_delta m_remaining;
    qint16 m_buffer[588*2*25];

public:
    RawReader(cdda::drive_handle &handle, cdda::block_addr start, cdda::block_addr_delta len, const TaskRunner::CancelToken &cancelToken)
        : ReaderBase(cancelToken), m_handle(handle), m_pos(start), m_remaining(len)
    {}

    bool eof() override { return m_remaining.delta_blocks <= 0; }
    bool read(qint16 **pBuffer, cdda::block_addr_delta *pBuflen) override
    {
        if (m_cancelToken.isCanceled())
            return false;

        auto count = std::min(cdda::block_addr_delta::from_lba(int(sizeof(m_buffer)/2352)), m_remaining);
        if (m_handle.read(m_buffer, m_pos, count))
        {
            *pBuffer = m_buffer;
            *pBuflen = count;
            m_pos += count;
            m_remaining -= count;
            return true;
        }
        else
        {
            *pBuffer = nullptr;
            *pBuflen = cdda::block_addr_delta::from_lba(0);
            return false;
        }
    }
    QString errorMessage() override
    {
        if (m_cancelToken.isCanceled())
        {
            return Extractor::tr("Canceled by user");
        }
        else
        {
            return m_handle.last_error();
        }
    }
};

class ParanoiaReader : public ReaderBase
{
private:
    cdda::drive_handle &m_handle;
    int m_blocksRemaining;
    cdrom_paranoia *m_paranoiaHandle;

    static long paranoiaReadfunc(void *userdata, void *buffer, long start, long len)
    {
        auto self = (ParanoiaReader*)userdata;

        if (self->m_cancelToken.isCanceled())
            return CDDA_ERROR_CANCELED;

        if (self->m_handle.read(buffer, cdda::block_addr::from_lba(int(start)), cdda::block_addr_delta::from_lba(int(len))))
            return len;
        else
            return -1L;
    }

public:
    ParanoiaReader(cdda::drive_handle &handle, cdda::block_addr start, cdda::block_addr_delta len, const TaskRunner::CancelToken &cancelToken)
        : ReaderBase(cancelToken), m_handle(handle)
    {
        m_paranoiaHandle = paranoia_init(&ParanoiaReader::paranoiaReadfunc,
                                         this,
                                         25,
                                         start.block, (start+len).block - 1); // FIXME! should use first and last sector of whole disc
        paranoia_modeset(m_paranoiaHandle, PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP);
        paranoia_set_range(m_paranoiaHandle, start.block, (start+len).block - 1);

        m_blocksRemaining = len.delta_blocks;
    }
    ~ParanoiaReader() override
    {
        paranoia_free(m_paranoiaHandle);
    }

    bool eof() override { return m_blocksRemaining <= 0; }
    bool read(qint16 **pBuffer, cdda::block_addr_delta *pBuflen) override
    {
        *pBuffer = paranoia_read(m_paranoiaHandle, nullptr);
        *pBuflen = cdda::block_addr_delta::from_lba(1);
        if (*pBuffer != nullptr)
        {
            m_blocksRemaining -= 1;
            return true;
        }
        else
        {
            return false;
        }
    }
    QString errorMessage() override
    {
        if (m_cancelToken.isCanceled())
        {
            return Extractor::tr("Canceled by user");
        }
        else
        {
            return m_handle.last_error();
        }
    }
};

QString extractTrack(cdda::drive_handle &handle,
                     const Extractor::TrackToExtract &track,
                     const QString &outdir,
                     const QString &format,
                     const TaskRunner::CancelToken &cancelToken,
                     const TaskRunner::ProgressToken &progressToken,
                     Extractor::ReadingMode mode)
{
    // initialize reader
    std::unique_ptr<ReaderBase> reader;

    if (mode == Extractor::READ_PARANOIA)
        reader.reset(new ParanoiaReader(handle, track.start, track.length, cancelToken));
    else
        reader.reset(new RawReader(handle, track.start, track.length, cancelToken));

    // initialize encoder
    Ringbuffer rb(100);
    QFuture<QString> encoderFuture = EncoderWriter::encodeAndWrite(outdir, format, track.length, track.metadata, &rb);
    TaskRunner::FutureFinishWaiter waiter(encoderFuture);

    // now extract data
    cdda::block_addr_delta processed_blocks { 0 };

    // loop for reading and encoding audio data
    while (!reader->eof())
    {
        qint16 *buf;
        cdda::block_addr_delta buflen { 0 };
        if (!reader->read(&buf, &buflen)) {
            encoderFuture.cancel();
            return Extractor::tr("Failed to read from CD: %1").arg(reader->errorMessage());
        }

        // write to ringbuffer
        for (int i = 0; i < buflen.delta_blocks; ++i) {
            qint16 *t = nullptr;

            while (!t) {
                t = rb.tryBeginWriteBlock(10000);
                if (encoderFuture.isFinished()) {
                    // encoder failed!
                    return encoderFuture.result();
                }
            }

            memcpy(t, &buf[588*2*i], 2352);

            rb.endWriteBlock();
        }

        processed_blocks += buflen;
        progressToken.reportProgressValueAndText(progressToken.progressValue() + buflen.delta_blocks,
                                                 Extractor::tr("Extracting Track %1 (%2)")
                                                             .arg(track.metadata.trackno)
                                                             .arg(processed_blocks.to_display()));
    }

    return encoderFuture.result();
}

} // anonymous namespace

QFuture<QString> Extractor::extract(const QString &device, const QString &outdir, const QString &format,
                                    const std::vector<Extractor::TrackToExtract> &tracks,
                                    ReadingMode mode)
{
    return TaskRunner::run([=](const TaskRunner::CancelToken &cancelToken, const TaskRunner::ProgressToken &progressToken) {
        // initialize progress reporting
        int totallen = 0;
        for (const auto &track : tracks) {
            totallen += track.length.delta_blocks;
        }

        progressToken.reportProgressRange(0, totallen);
        progressToken.reportProgressValueAndText(1, tr("Initializing..."));

        // open drive
        cdda::drive_handle handle = cdda::drive_handle::open(device);
        if (!handle) {
            return tr("Could not open %1: %2").arg(device).arg(handle.last_error());
        }

        // extract tracks
        QElapsedTimer tTotal;
        tTotal.start();
        for (const auto &track : tracks) {
            QElapsedTimer t;
            t.start();

            QString e = extractTrack(handle, track, outdir, format, cancelToken, progressToken, mode);
            if (e.length())
                return e;

            qDebug() << "Extracting track" << track.metadata.trackno << "took" << t.elapsed() << "ms";
        }

        qDebug() << "Whole extraction took" << tTotal.elapsed() << "ms";

        return QString();
    });
}
