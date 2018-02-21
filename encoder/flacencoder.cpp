#include "flacencoder.h"

#include <FLAC/stream_encoder.h>
#include <QIODevice>

namespace {

FLAC__StreamEncoderWriteStatus writeToQIODevice(const FLAC__StreamEncoder *encoder,
                                                const FLAC__byte buffer[],
                                                size_t bytes,
                                                unsigned samples,
                                                unsigned current_frame,
                                                void *client_data)
{
    Q_UNUSED(samples);
    Q_UNUSED(current_frame);
    Q_UNUSED(encoder);

    QIODevice *dev = (QIODevice*)client_data;

    if (dev->write((char*)buffer, bytes) == (qint64)bytes)
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    else
        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
}

FLAC__StreamEncoderSeekStatus seekOnQIODevice(const FLAC__StreamEncoder *, FLAC__uint64 absolute_byte_offset, void *client_data)
{
    QIODevice *dev = (QIODevice*)client_data;

    if (dev->seek(absolute_byte_offset))
        return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
    else
        return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
}

FLAC__StreamEncoderTellStatus tellOnQIODevice(const FLAC__StreamEncoder *, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
    QIODevice *dev = (QIODevice*)client_data;
    *absolute_byte_offset = dev->pos();
    return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
}

} // anonymous namespace

namespace Encoder {

FlacEncoder::FlacEncoder()
{

}

FlacEncoder::~FlacEncoder()
{
    if (m_encoder)
    {
        FLAC__stream_encoder_delete(m_encoder);
        m_encoder = nullptr;
    }
}

} // namespace Encoder


bool Encoder::FlacEncoder::initialize(QIODevice *device, qint64, const cdda::track_metadata &)
{
    m_encoder = FLAC__stream_encoder_new();
    FLAC__stream_encoder_set_streamable_subset(m_encoder, true);
    FLAC__stream_encoder_set_channels(m_encoder, 2);
    FLAC__stream_encoder_set_bits_per_sample(m_encoder, 16);
    FLAC__stream_encoder_set_sample_rate(m_encoder, 44100);

    auto status = FLAC__stream_encoder_init_stream(m_encoder, &writeToQIODevice, &seekOnQIODevice, &tellOnQIODevice, nullptr, device);

    if (status != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
    {
        m_error = QObject::tr("FLAC Error: %1").arg(QString::fromUtf8(FLAC__StreamEncoderInitStatusString[status]));
        return false;
    }

    return true;
}

bool Encoder::FlacEncoder::feed(const qint16 *buf16, qint64 numSamples)
{
    // flac wants samples as int32_t  -.-

    /*qint32 buf32[2048];

    while (numSamples > 0)
    {
        qint64 step = std::min(qint64(sizeof(buf32)/sizeof(buf32[0])/2), numSamples);
        for (qint64 i = 0; i < step; ++i)
        {
            buf32[2*i]   = buf16[2*i];
            buf32[2*i+1] = buf16[2*i+1];
        }

        if (!FLAC__stream_encoder_process_interleaved(m_encoder, buf32, unsigned(step)))
        {
            auto status = FLAC__stream_encoder_get_state(m_encoder);
            m_error = QObject::tr("FLAC Error: %1").arg(QString::fromUtf8(FLAC__StreamEncoderInitStatusString[status]));
            return false;
        }

        numSamples -= step;
    }*/
    for (qint64 i = 0; i < numSamples; ++i)
    {
        qint32 buf32[] = { buf16[2*i], buf16[2*i+1] };
        if (!FLAC__stream_encoder_process_interleaved(m_encoder, buf32, 1))
        {
            auto status = FLAC__stream_encoder_get_state(m_encoder);
            m_error = QObject::tr("FLAC Error: %1").arg(QString::fromUtf8(FLAC__StreamEncoderStateString[status]));
            return false;
        }
    }

    return true;

}

bool Encoder::FlacEncoder::finish()
{
    bool ok = FLAC__stream_encoder_finish(m_encoder);

    if (!ok)
    {
        auto status = FLAC__stream_encoder_get_state(m_encoder);
        m_error = QObject::tr("FLAC Error: %1").arg(QString::fromUtf8(FLAC__StreamEncoderStateString[status]));
    }

    FLAC__stream_encoder_delete(m_encoder);
    m_encoder = nullptr;
    return ok;
}

QString Encoder::FlacEncoder::errorText()
{
    return m_error;
}
