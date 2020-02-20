#ifndef LAMEENCODER_H
#define LAMEENCODER_H

#include "abstractencoder.h"

#include <QCoreApplication>
#include <lame.h>

namespace Encoder {

class LameEncoder : public AbstractEncoder
{
    Q_DECLARE_TR_FUNCTIONS(LameEncoder)

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
    lame_global_flags *m_gfp { nullptr };

    QIODevice *m_device { nullptr };
    QString m_error;
};

} // namespace Encoder

#endif // LAMEENCODER_H
