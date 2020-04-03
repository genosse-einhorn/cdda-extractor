// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
