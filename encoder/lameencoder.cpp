#include "lameencoder.h"
#include <QIODevice>

#include <lame.h>

namespace {

template<typename T> QString withBom(const T &s)
{
    return QStringLiteral(u"\uFEFF%1").arg(s);
}

} // anonymous namespace

namespace Encoder {

LameEncoder::LameEncoder()
{
}

LameEncoder::~LameEncoder()
{
    if (m_gfp)
    {
        lame_close(m_gfp);
        m_gfp = nullptr;
    }
}

bool LameEncoder::initialize(QIODevice *device, qint64 numSamples, const cdda::track_metadata &metadata)
{
    m_device = device;

    if (m_gfp)
        lame_close(m_gfp);

    m_gfp = lame_init();

    lame_set_quality(m_gfp, 2);
    lame_set_mode(m_gfp, JOINT_STEREO);
    lame_set_brate(m_gfp, 192);
    lame_set_in_samplerate(m_gfp, 44100);
    lame_set_num_channels(m_gfp, 2);
    lame_set_num_samples(m_gfp, (unsigned long)numSamples);

    id3tag_init(m_gfp);
    id3tag_v2_only(m_gfp);
    id3tag_pad_v2(m_gfp);

    // set ID3 tags
    if (metadata.coverPng.size())
        id3tag_set_albumart(m_gfp, metadata.coverPng.constData(), metadata.coverPng.size());
    if (metadata.album.size())
        id3tag_set_textinfo_utf16(m_gfp, "TALB", withBom(metadata.album).utf16());
    if (metadata.artist.size())
        id3tag_set_textinfo_utf16(m_gfp, "TPE1", withBom(metadata.artist).utf16());
    if (metadata.genre.size())
        id3tag_set_textinfo_utf16(m_gfp, "TCON", withBom(metadata.genre).utf16());
    if (metadata.composer.size())
        id3tag_set_textinfo_utf16(m_gfp, "TCOM", withBom(metadata.composer).utf16());
    if (metadata.isrc.size())
        id3tag_set_textinfo_utf16(m_gfp, "TSRC", withBom(metadata.isrc).utf16());
    if (metadata.title.size())
        id3tag_set_textinfo_utf16(m_gfp, "TIT2", withBom(metadata.title).utf16());
    if (metadata.trackno)
        id3tag_set_textinfo_utf16(m_gfp, "TRCK", withBom(metadata.trackno).utf16());
    if (metadata.year.size())
        id3tag_set_textinfo_utf16(m_gfp, "TYER", withBom(metadata.year).utf16());

    lame_set_write_id3tag_automatic(m_gfp, 0);

    lame_init_params(m_gfp);

    // write out id3 tag now
    std::vector<unsigned char> buf;
    buf.resize(1);

    auto bufsize = lame_get_id3v2_tag(m_gfp, &buf[0], buf.size());
    buf.resize(bufsize);
    lame_get_id3v2_tag(m_gfp, &buf[0], buf.size());

    if (device->write((const char*)&buf[0], qint64(buf.size())) != qint64(buf.size()))
    {
        m_error = tr("I/O error: %1").arg(device->errorString());
        return false;
    }

    return true;
}

bool LameEncoder::feed(const qint16 *buf, qint64 numSamples)
{
    std::vector<unsigned char> mp3buf;
    mp3buf.resize((numSamples / 44100 + 1) * (192000/8) + 7200);

    int c = lame_encode_buffer_interleaved(m_gfp, (short*)buf, (int)numSamples, &mp3buf[0], (int)mp3buf.size());

    if (c < 0)
    {
        m_error = tr("MP3/LAME error %1").arg(c);
        return false;
    }
    else
    {
        if (m_device->write((const char *)&mp3buf[0], c) != c)
        {
            m_error = tr("I/O error: %1").arg(m_device->errorString());

            return false;
        }
    }

    return true;
}

bool LameEncoder::finish()
{
    unsigned char lastbits[7200];

    int c = lame_encode_flush(m_gfp, lastbits, sizeof(lastbits));

    if (c < 0)
    {
        m_error = tr("MP3/LAME error %1").arg(c);
        return false;
    }
    else
    {
        if (m_device->write((const char *)lastbits, c) != c)
        {
            m_error = tr("I/O error: %1").arg(m_device->errorString());

            return false;
        }
    }

    return true;
}

QString LameEncoder::errorText()
{
    return m_error;
}

} // namespace Encoder
