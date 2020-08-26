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

#include "encoderwriter.h"

#include <QDir>

#include "tasklib/taskrunner.h"
#include "encoder/flacencoder.h"
#include "encoder/lameencoder.h"
#include "encoder/wavencoder.h"
#include "fileutil.h"

QFuture<QString> EncoderWriter::encodeAndWrite(const QString &outdir, const QString &format, cdda::block_addr_delta track_len, const cdda::track_metadata &metadata, Ringbuffer *rb)
{
    return TaskRunner::run([=](const TaskRunner::CancelToken &cancel, const TaskRunner::ProgressToken &progress) {
        // create directories
        if (!QDir(outdir).mkpath(QStringLiteral(".")))
            return tr("Could not create directory %1").arg(outdir);


        // create file without overwriting files already there
        QString basename = FileUtil::sanitizeFilename(QStringLiteral("%1 - %2")
                .arg(metadata.trackno, 2, 10, QLatin1Char('0'))
                .arg(metadata.title.size() ? metadata.title : QStringLiteral("Track %1").arg(metadata.trackno)));

        QString filename = QStringLiteral("%1/%2.%3").arg(outdir, basename, format);

        QString error;
        std::unique_ptr<QIODevice> device(FileUtil::createFileExclusive(filename, &error));
        unsigned c = 0;
        while (!device.get() && !error.size())
        {
            // file exists - try next one
            ++c;
            filename = QStringLiteral("%1/%2 (%4).%3").arg(outdir, basename, format).arg(c);
            device.reset(FileUtil::createFileExclusive(filename, &error));
        }

        if (!device.get())
            return tr("Could not create file %1: %2").arg(filename, error);

        // create encoder instance
        std::unique_ptr<Encoder::AbstractEncoder> encoder;

        if (format == QStringLiteral("flac"))
            encoder.reset(new Encoder::FlacEncoder());
        else if (format == QStringLiteral("mp3"))
            encoder.reset(new Encoder::LameEncoder());
        else
            encoder.reset(new Encoder::WavEncoder());

        // initialize encoder
        if (!encoder->initialize(device.get(), track_len.delta_blocks*588, metadata))
            return tr("Failed to initialize encoder: %1").arg(encoder->errorText());

        // really simple pre-gap detection: chop off silence from the end
        qint64 trailingSilentSamples = 0;

        for (int i = 0; i < track_len.delta_blocks; ++i) {
            qint16 *buf = nullptr;
            while (!buf) {
                buf = rb->tryBeginReadBlock(2000);
                if (cancel.isCanceled())
                    return tr("Canceled");
            }

            // pre-gap detection
            qint64 samples = 588;
            qint64 silentSamplesThisBuf = 0;
            while (samples > 0 && !buf[2*samples-1] && !buf[2*samples-2])
            {
                samples--;
                silentSamplesThisBuf++;
            }

            if (samples)
            {
                // write buffered silent samples
                while (trailingSilentSamples > 0)
                {
                    qint16 silentBuf[200] = {};
                    qint64 n = std::min(trailingSilentSamples, qint64(sizeof(silentBuf)/sizeof(silentBuf[0]))/2);
                    if (!encoder->feed(silentBuf, n))
                        return tr("Failed while encoding audio: %1").arg(encoder->errorText());

                    trailingSilentSamples -= n;
                }

                // encode it
                if (!encoder->feed(buf, samples))
                    return tr("Failed while encoding audio: %1").arg(encoder->errorText());
            }

            trailingSilentSamples += silentSamplesThisBuf;

            rb->endReadBlock();
        }

        // finalize encoding
        if (!encoder->finish())
            return tr("Failed to finalize encoding: %1").arg(encoder->errorText());

        return QString();
    });
}
