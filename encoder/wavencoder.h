#ifndef WAVENCODER_H
#define WAVENCODER_H

#include "abstractencoder.h"

#include <QCoreApplication>

namespace Encoder {

class WavEncoder : public Encoder::AbstractEncoder
{
    Q_DECLARE_TR_FUNCTIONS(WavEncoder)

public:
    WavEncoder();
    ~WavEncoder() override;

    // AbstractEncoder interface
public:
    bool initialize(QIODevice *device, qint64 numSamples, const cdda::track_metadata &) override;
    bool feed(const qint16 *buf, qint64 numSamples) override;
    bool finish() override;
    QString errorText() override;

private:
    QString m_errorText;
    QIODevice *m_device;
};

} // namespace Encoder

#endif // WAVENCODER_H
