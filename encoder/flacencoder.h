#ifndef FLACENCODER_H
#define FLACENCODER_H

#include "abstractencoder.h"

#include <FLAC/stream_encoder.h>
#include <vector>

#include <QCoreApplication>

namespace Encoder {

class FlacEncoder : public AbstractEncoder
{
    Q_DECLARE_TR_FUNCTIONS(FlacEncoder)

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

    std::vector<FLAC__StreamMetadata *> m_metadata;
};

} // namespace Encoder

#endif // FLACENCODER_H
