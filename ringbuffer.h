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

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QSemaphore>

class Ringbuffer
{
    Q_DISABLE_COPY_MOVE(Ringbuffer);
public:
    Ringbuffer(unsigned numblocks);
    ~Ringbuffer();

    qint16 *beginWriteBlock();
    qint16 *tryBeginWriteBlock(int timeout);
    void endWriteBlock();

    qint16 *beginReadBlock();
    qint16 *tryBeginReadBlock(int timeout);
    void endReadBlock();

private:
    qint16 *m_buf;
    unsigned m_nblocks;
    unsigned m_readCounter;
    unsigned m_writeCounter;
    QSemaphore m_blocksFree;
    QSemaphore m_blocksUsed;
};

#endif // RINGBUFFER_H
