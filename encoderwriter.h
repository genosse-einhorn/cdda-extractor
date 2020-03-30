#ifndef ENCODERWRITER_H
#define ENCODERWRITER_H

#include <QFuture>
#include <QString>
#include <QApplication>
#include "ringbuffer.h"
#include "libcdda/toc.h"

class EncoderWriter
{
    EncoderWriter() = delete;
    Q_DECLARE_TR_FUNCTIONS(EncoderWriter);
public:
    static QFuture<QString> encodeAndWrite(const QString &dir, const QString &format, cdda::block_addr_delta track_len, const cdda::track_metadata &metadata, Ringbuffer *rb);
};


#endif // ENCODERWRITER_H
