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

#include "ringbuffer.h"

Ringbuffer::Ringbuffer(unsigned numblocks)
    : m_buf(new qint16[588*2*numblocks]),
      m_nblocks(numblocks),
      m_readCounter(0), m_writeCounter(0),
      m_blocksFree(int(numblocks)),
      m_blocksUsed(0)
{
}

Ringbuffer::~Ringbuffer()
{
    delete[] m_buf;
}

qint16 *Ringbuffer::beginWriteBlock()
{
    return tryBeginWriteBlock(-1);
}

qint16 *Ringbuffer::tryBeginWriteBlock(int timeout)
{
    if (m_blocksFree.tryAcquire(1, timeout)) {
        return &m_buf[588*2*m_writeCounter];
    } else {
        return nullptr;
    }
}

void Ringbuffer::endWriteBlock()
{
    m_blocksUsed.release();
    m_writeCounter = (m_writeCounter + 1) % m_nblocks;
}

qint16 *Ringbuffer::beginReadBlock()
{
    return tryBeginReadBlock(-1);
}

qint16 *Ringbuffer::tryBeginReadBlock(int timeout)
{
    if (m_blocksUsed.tryAcquire(1, timeout)) {
        return &m_buf[588*2*m_readCounter];
    } else {
        return nullptr;
    }
}

void Ringbuffer::endReadBlock()
{
    m_blocksFree.release();
    m_readCounter = (m_readCounter + 1) % m_nblocks;
}
