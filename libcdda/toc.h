#ifndef CDDA_TOC_H
#define CDDA_TOC_H

#include <QString>
#include <QMetaType>
#include <vector>

namespace cdda {

struct block_addr_delta {
    int delta_blocks;

    int minute() const { return ((delta_blocks) / 75 / 60); }

    int second() const { return ((delta_blocks) / 75) % 60; }

    int frame() const {  return delta_blocks % 75; }

    static block_addr_delta from_lba(int index)
    {
        block_addr_delta a { index };
        return a;
    }

    static block_addr_delta from_msf(int m, int s, int f)
    {
        return from_lba(f + (s + m * 60) * 75);
    }

    QString to_display() const
    {
        return QStringLiteral("%1:%2.%3")
                .arg(minute(), 2, 10, QChar(u'0'))
                .arg(second(), 2, 10, QChar(u'0'))
                .arg(frame(), 2, 10, QChar(u'0'));
    }

    bool operator==(block_addr_delta other) const { return delta_blocks == other.delta_blocks; }
    bool operator!=(block_addr_delta other) const { return !(*this == other); }
    bool operator<(block_addr_delta other) const { return delta_blocks < other.delta_blocks; }
    bool operator<=(block_addr_delta other) const { return !(other < *this); }
    bool operator>(block_addr_delta other) const { return other < *this; }
    bool operator>=(block_addr_delta other) const { return !(*this < other); }
    block_addr_delta &operator+=(block_addr_delta other) { delta_blocks += other.delta_blocks; return *this; }
    block_addr_delta &operator-=(block_addr_delta other) { delta_blocks -= other.delta_blocks; return *this; }
};

struct block_addr {
    int block;

    int minute() const { return ((block + 150) / 75 / 60); }

    int second() const { return ((block + 150) / 75) % 60; }

    int frame() const {  return block % 75; }

    static block_addr from_lba(int index)
    {
        block_addr a { index };
        return a;
    }

    static block_addr from_msf(int m, int s, int f)
    {
        return from_lba(f + (s + m * 60) * 75 - 150);
    }

    bool operator==(block_addr other) const { return block == other.block; }
    bool operator!=(block_addr other) const { return !(*this == other); }
    bool operator<(block_addr other) const { return block < other.block; }
    bool operator<=(block_addr other) const { return !(other < *this); }
    bool operator>(block_addr other) const { return other < *this; }
    bool operator>=(block_addr other) const { return !(*this < other); }
    block_addr &operator+=(block_addr_delta other) { block += other.delta_blocks; return *this; }
    block_addr &operator-=(block_addr_delta other) { block -= other.delta_blocks; return *this; }

    QString to_display() const
    {
        return QStringLiteral("%1:%2.%3")
                .arg(minute(), 2, 10, QChar(u'0'))
                .arg(second(), 2, 10, QChar(u'0'))
                .arg(frame(), 2, 10, QChar(u'0'));
    }
};

inline block_addr operator+(block_addr a, block_addr_delta b) { return block_addr::from_lba(a.block + b.delta_blocks); }
inline block_addr operator+(block_addr_delta a, block_addr b) { return block_addr::from_lba(a.delta_blocks + b.block); }
inline block_addr_delta operator+(block_addr_delta a, block_addr_delta b) { a += b; return a; }

inline block_addr operator-(block_addr a, block_addr_delta b) { a -= b; return a; }
inline block_addr_delta operator-(block_addr a, block_addr b) { return block_addr_delta::from_lba(a.block - b.block); }
inline block_addr_delta operator-(block_addr_delta a, block_addr_delta b) { a -= b; return a; }

struct toc_track {
    // cd session
    unsigned char session;

    // track number
    int index;

    block_addr start;
    block_addr_delta length;

    // stuff from the scsi response
    unsigned char adr;
    unsigned char control;

    // subchannel data
    QString isrc;

    // TODO: CD-TEXT

    bool is_audio() const { return (control & 4) == 0; }
};

struct toc {
    std::vector<toc_track> tracks;
    QString catalog;

    bool is_valid() { return tracks.size() > 0; }
};

struct track_metadata {
    int trackno;
    QString title;
    QString artist;
    QString composer;
    QString album;
    QString genre;
    QString year;
    QString discNo;
    QString isrc;
};

QString calculate_musicbrainz_discid(const toc &toc);

} // namespace cdda

Q_DECLARE_METATYPE(cdda::block_addr)
Q_DECLARE_METATYPE(cdda::block_addr_delta)
Q_DECLARE_METATYPE(cdda::toc_track)
Q_DECLARE_METATYPE(cdda::track_metadata)

#endif // CDDA_TOC_H
