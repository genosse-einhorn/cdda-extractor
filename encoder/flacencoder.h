#ifndef FLACENCODER_H
#define FLACENCODER_H

#include "abstractencoder.h"

#include <FLAC/stream_encoder.h>

namespace Encoder {

class FlacEncoder : public AbstractEncoder
{
public:
    FlacEncoder();
    ~FlacEncoder();

    // AbstractEncoder interface
public:
    bool initialize(QIODevice *device, qint64 numSamples, const cdda::track_metadata &metadata) override;
    bool feed(const qint16 *buf, qint64 numSamples) override;
    bool finish() override;
    QString errorText() override;

private:
    FLAC__StreamEncoder *m_encoder;
    QString m_error;
};

} // namespace Encoder

#endif // FLACENCODER_H
