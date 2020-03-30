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
