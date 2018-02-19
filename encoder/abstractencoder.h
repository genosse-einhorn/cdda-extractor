#ifndef ABSTRACTENCODER_H
#define ABSTRACTENCODER_H

#include "libcdda/toc.h"

class QIODevice;

namespace Encoder {

class AbstractEncoder
{
public:
    AbstractEncoder();
    virtual ~AbstractEncoder();

    virtual bool initialize(QIODevice *device, qint64 numSamples, const cdda::track_metadata &metadata) = 0;
    virtual bool feed(const qint16 *buf, qint64 numSamples) = 0;
    virtual bool finish() = 0;
    virtual QString errorText() = 0;
};

} // namespace Encoder

#endif // ABSTRACTENCODER_H
