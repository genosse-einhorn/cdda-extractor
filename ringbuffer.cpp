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
