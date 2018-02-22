#include "flacencoder.h"

#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>
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

    for (auto *meta : m_metadata)
    {
        FLAC__metadata_object_delete(meta);
    }
    m_metadata.clear();
}

} // namespace Encoder


bool Encoder::FlacEncoder::initialize(QIODevice *device, qint64, const cdda::track_metadata &meta)
{
    // initialize metadata
    for (auto *o : m_metadata)
        FLAC__metadata_object_delete(o);
    m_metadata.clear();

    // vorbis comment
    FLAC__StreamMetadata *vorbiscomm = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    m_metadata.push_back(vorbiscomm);

    if (meta.album.size())
    {
        QByteArray c = QStringLiteral("ALBUM=%1").arg(meta.album).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.artist.size())
    {
        QByteArray c = QStringLiteral("ARTIST=%1").arg(meta.artist).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.title.size())
    {
        QByteArray c = QStringLiteral("TITLE=%1").arg(meta.title).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.composer.size())
    {
        QByteArray c = QStringLiteral("COMPOSER=%1").arg(meta.composer).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.discNo.size())
    {
        QByteArray c = QStringLiteral("DISCNUMBER=%1").arg(meta.discNo).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.genre.size())
    {
        QByteArray c = QStringLiteral("GENRE=%1").arg(meta.genre).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.isrc.size())
    {
        QByteArray c = QStringLiteral("ISRC=%1").arg(meta.isrc).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.trackno)
    {
        QByteArray c = QStringLiteral("TRACKNUMBER=%1").arg(meta.trackno).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.tracktotal)
    {
        QByteArray c = QStringLiteral("TRACKTOTAL=%1").arg(meta.tracktotal).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (meta.year.size())
    {
        QByteArray c = QStringLiteral("DATE=%1").arg(meta.year).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    // picture
    if (meta.coverPng.size())
    {
        FLAC__StreamMetadata *picture = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
        FLAC__metadata_object_picture_set_mime_type(picture, (char*)"image/png", true);
        FLAC__metadata_object_picture_set_data(picture, (FLAC__byte*)meta.coverPng.data(), (FLAC__uint32)meta.coverPng.size(), true);
        m_metadata.push_back(picture);
    }

    m_encoder = FLAC__stream_encoder_new();
    FLAC__stream_encoder_set_streamable_subset(m_encoder, true);
    FLAC__stream_encoder_set_channels(m_encoder, 2);
    FLAC__stream_encoder_set_bits_per_sample(m_encoder, 16);
    FLAC__stream_encoder_set_sample_rate(m_encoder, 44100);
    FLAC__stream_encoder_set_metadata(m_encoder, &m_metadata[0], (unsigned)m_metadata.size());

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
