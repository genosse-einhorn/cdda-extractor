#include "toc.h"

#include <cstring>
#include <QCryptographicHash>

static int block_addr_typeid = qRegisterMetaType<cdda::block_addr>();
static int block_addr_delta_typeid = qRegisterMetaType<cdda::block_addr_delta>();
static int toc_track_typeid = qRegisterMetaType<cdda::toc_track>();
static int track_metadata_typeid = qRegisterMetaType<cdda::track_metadata>();

QString cdda::calculate_musicbrainz_discid(const toc &toc)
{
    const char hexmap[] = "0123456789ABCDEF";

    QByteArray arr(804, '0');
    quint8 firstTrackNo = 99;
    quint8 lastTrackNo = 0;
    quint32 leadOutOffset = 0;

    for (auto track : toc.tracks)
    {
        if (!track.is_audio())
            continue;
        if (track.index < 1)
            continue; // ignore synthesized hidden track

        if (track.index < firstTrackNo)
            firstTrackNo = quint8(track.index);
        if (track.index > lastTrackNo)
        {
            lastTrackNo = quint8(track.index);
            leadOutOffset = (track.start + track.length).block + 150;
        }

        int i = 4 + track.index*8;
        quint32 offset = track.start.block + 150;

        arr[i  ] = hexmap[(offset & 0xf0000000) >> 28];
        arr[i+1] = hexmap[(offset & 0x0f000000) >> 24];
        arr[i+2] = hexmap[(offset & 0x00f00000) >> 20];
        arr[i+3] = hexmap[(offset & 0x000f0000) >> 16];
        arr[i+4] = hexmap[(offset & 0x0000f000) >> 12];
        arr[i+5] = hexmap[(offset & 0x00000f00) >> 8];
        arr[i+6] = hexmap[(offset & 0x000000f0) >> 4];
        arr[i+7] = hexmap[(offset & 0x0000000f)];
    }

    arr[0] = hexmap[(firstTrackNo & 0xf0) >> 4];
    arr[1] = hexmap[(firstTrackNo & 0x0f)];
    arr[2] = hexmap[(lastTrackNo & 0xf0) >> 4];
    arr[3] = hexmap[(lastTrackNo & 0x0f)];

    arr[4]  = hexmap[(leadOutOffset & 0xf0000000) >> 28];
    arr[5]  = hexmap[(leadOutOffset & 0x0f000000) >> 24];
    arr[6]  = hexmap[(leadOutOffset & 0x00f00000) >> 20];
    arr[7]  = hexmap[(leadOutOffset & 0x000f0000) >> 16];
    arr[8]  = hexmap[(leadOutOffset & 0x0000f000) >> 12];
    arr[9]  = hexmap[(leadOutOffset & 0x00000f00) >> 8];
    arr[10] = hexmap[(leadOutOffset & 0x000000f0) >> 4];
    arr[11] = hexmap[(leadOutOffset & 0x0000000f)];

    QByteArray hash = QCryptographicHash::hash(arr, QCryptographicHash::Sha1);
    QByteArray base64 = hash.toBase64(QByteArray::Base64Encoding);

    // replace some characters to be compatible to musicbrainz base64
    for (int i = 0; i < base64.length(); ++i)
    {
        switch (base64[i])
        {
        case '+':
            base64[i] = '.';
            break;
        case '/':
            base64[i] = '_';
            break;
        case '=':
            base64[i] = '-';
            break;
        }
    }

    return QString::fromLatin1(base64);
}
