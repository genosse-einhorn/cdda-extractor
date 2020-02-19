#include "wavencoder.h"

#include <QIODevice>
#include <cstring>

namespace Encoder {

WavEncoder::WavEncoder()
{

}

WavEncoder::~WavEncoder()
{
}

} // namespace Encoder


bool Encoder::WavEncoder::initialize(QIODevice *device, qint64 numSamples, const cdda::track_metadata &)
{
    m_device = device;

    // write WAV header
#pragma pack(push, 1)
    struct {
        char ChunkID[4];
        quint32 ChunkSize;
        char Format[4];
        struct {
            char ChunkID[4];
            quint32 ChunkSize;
            quint16 AudioFormat;
            quint16 NumChannels;
            quint32 SampleRate;
            quint32 ByteRate;
            quint16 BlockAlign;
            quint16 BitsPerSample;
        } fmt;
        struct {
            char ChunkID[4];
            quint32 ChunkSize;
            /* data... */
        } data;
    } header;
#pragma pack(pop)
    std::memset(&header, 0, sizeof(header));
    std::memcpy(header.ChunkID, "RIFF", 4);
    header.ChunkSize = quint32(sizeof(header)-8+numSamples*4);
    std::memcpy(header.Format, "WAVE", 4);
    std::memcpy(header.fmt.ChunkID, "fmt ", 4);
    header.fmt.ChunkSize = sizeof(header.fmt)-8;
    header.fmt.AudioFormat = 1; // PCM;
    header.fmt.NumChannels = 2;
    header.fmt.SampleRate = 44100;
    header.fmt.ByteRate = 44100 * 4;
    header.fmt.BlockAlign = 4;
    header.fmt.BitsPerSample = 16;
    std::memcpy(header.data.ChunkID, "data", 4);
    header.data.ChunkSize = quint32(numSamples * 4);

    qint64 written = m_device->write((char*)&header, sizeof(header));
    if (written != sizeof(header))
    {
        m_errorText = tr("I/O error: %1").arg(m_device->errorString());
        return false;
    }

    return true;
}

bool Encoder::WavEncoder::feed(const qint16 *buf, qint64 numSamples)
{
    qint64 written = m_device->write((const char*)buf, numSamples * 4);
    if (written != numSamples*4)
    {
        m_errorText = tr("I/O error: %1").arg(m_device->errorString());
        return false;
    }

    return true;
}

bool Encoder::WavEncoder::finish()
{
    return true;
}

QString Encoder::WavEncoder::errorText()
{
    return m_errorText;
}
