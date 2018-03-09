#ifndef LAMEENCODER_H
#define LAMEENCODER_H

#include "abstractencoder.h"
#include "lame_backend.h"

namespace Encoder {

class LameEncoder : public AbstractEncoder
{
public:
    LameEncoder();
    ~LameEncoder();

    // AbstractEncoder interface
public:
    bool initialize(QIODevice *device, qint64 numSamples, const cdda::track_metadata &metadata) override;
    bool feed(const qint16 *buf, qint64 numSamples) override;
    bool finish() override;
    QString errorText() override;

private:
    LameFuncTable *m_lame;
    LameHandle m_gfp;

    QIODevice *m_device;
    QString m_error;
};

} // namespace Encoder

#endif // LAMEENCODER_H
