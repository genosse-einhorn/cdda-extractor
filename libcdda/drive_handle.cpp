#include "drive_handle.h"

#include <algorithm>
#include <cstring>
#include <cmath>
#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/sg.h>
#endif
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#include <winioctl.h>
#include <ntddcdrm.h>
#include <ntddscsi.h>
#endif

#include <QFileInfo>
#include <QProcess>
#include <QDebug>

#include "sense.h"
#include "os_util.h"

namespace cdda {

void drive_handle::cleanup()
{
#ifdef Q_OS_LINUX
    if (m_fd >= 0)
        ::close(m_fd);
#endif
#ifdef Q_OS_WIN32
    if (m_handle != 0 && m_handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_handle);
#endif
}

drive_handle::drive_handle()
{
}

drive_handle::drive_handle(drive_handle &&other)
{
    using std::swap;
#ifdef Q_OS_LINUX
    swap(m_fd, other.m_fd);
#endif
#ifdef Q_OS_WIN32
    swap(m_handle, other.m_handle);
#endif
    swap(m_lastErr, other.m_lastErr);
    swap(m_deviceName, other.m_deviceName);
}

drive_handle::~drive_handle()
{
    cleanup();
}

drive_handle &
drive_handle::operator=(drive_handle &&other)
{
    cleanup();

#ifdef Q_OS_LINUX
    m_fd = other.m_fd;
    other.m_fd = -1;
#endif
#ifdef Q_OS_WIN32
    m_handle = other.m_handle;
    other.m_handle = 0;
#endif
    m_lastErr = other.m_lastErr;
    m_deviceName = other.m_deviceName;

    return *this;
}

drive_handle drive_handle::open(const QString &drive)
{
    drive_handle ret;

    ret.m_deviceName = drive;

#if defined(Q_OS_LINUX)
    ret.m_fd = ::open(drive.toUtf8(), O_RDONLY | O_NONBLOCK);
    if (!ret.ok())
        ret.m_lastErr = os_error_to_str(errno);
#elif defined(Q_OS_WIN32)
    ret.m_handle = CreateFile((wchar_t *)drive.utf16(), GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!ret.ok())
        ret.m_lastErr = os_error_to_str(GetLastError());
#else
#error "Support for your OS is missing!"
#endif

    return ret;
}

bool drive_handle::exec_scsi_command(void *cmd, qint64 cmdlen, quint8 *bufp, qint64 buflen)
{
#if defined(Q_OS_LINUX)
    // linux SG_IO
    struct sg_io_hdr hdr;
    sense_data sense;
    std::memset(&hdr, 0, sizeof(hdr));
    std::memset(&sense, 0, sizeof(sense));

    hdr.interface_id = 'S';
    hdr.cmdp = (unsigned char*)cmd;
    hdr.cmd_len = (unsigned char)cmdlen;
    hdr.dxferp = bufp;
    hdr.dxfer_len = (unsigned int)buflen;
    hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    hdr.sbp = (unsigned char*)&sense;
    hdr.mx_sb_len = sizeof(sense);
    hdr.timeout = 10000;

    int ret = ioctl(m_fd, SG_IO, &hdr);
    if (ret < 0)
    {
        m_lastErr = os_error_to_str(errno);
        return false;
    }

    if (hdr.status)
    {
        m_lastErr = QStringLiteral("SCSI Error: %2").arg(sense_to_string(sense));
        return false;
    }

    return true;
#elif defined(Q_OS_WIN32)
    struct {
        SCSI_PASS_THROUGH_DIRECT s;
        sense_data sense;
    } s;
    std::memset(&s, 0, sizeof(s));
    s.s.Length = sizeof(s.s);
    std::memcpy(s.s.Cdb, cmd, std::min(cmdlen, qint64(16)));
    s.s.CdbLength = UCHAR(cmdlen);
    s.s.TimeOutValue = 30;
    s.s.DataBuffer = bufp;
    s.s.DataTransferLength = ULONG(buflen);
    s.s.SenseInfoLength = sizeof(s.sense);
    s.s.SenseInfoOffset = (char*)&s.sense - (char*)&s;

    DWORD bytes;
    BOOL ret = DeviceIoControl(m_handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, &s, sizeof(s), &s, sizeof(s), &bytes, nullptr);
    if (!ret)
    {
        m_lastErr = os_error_to_str(GetLastError());
        return false;
    }

    if (s.s.ScsiStatus)
    {
        m_lastErr = QStringLiteral("SCSI Status %1 / Error: %2").arg(s.s.ScsiStatus).arg(sense_to_string(s.sense));
        return false;
    }

    return true;
#else
#error "Missing support for your favorite OS"
#endif
}

bool drive_handle::eject()
{
#ifdef Q_OS_LINUX
    // the eject utility will try a whole bunch of ioctls
    // we could maybe do just as good by reimplementing eject here, but why would we do that?
    if (QProcess::execute(QStringLiteral("eject %1").arg(device_name())) == 0)
        return true;
#endif

    // scsi START STOP UNIT
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 immed : 1;
        quint8 reserv1 : 7;
        quint8 reserv2;
        quint8 formatlayerno : 2;
        quint8 reserv3 : 6;
        quint8 start : 1;
        quint8 loej : 1;
        quint8 fl : 1;
        quint8 reserv4 : 1;
        quint8 powercond : 4;
        quint8 control;
    } cdb = {};
#pragma pack(pop)

    cdb.opcode = 0x1b;
    cdb.start = 0;
    cdb.loej = 1;

    return exec_scsi_command(&cdb, sizeof(cdb), nullptr, 0);
}

bool drive_handle::close_tray()
{
#ifdef Q_OS_LINUX
    // the eject utility will try a whole bunch of ioctls
    // and will usually work better than what we could implement
    if (QProcess::execute(QStringLiteral("eject -t %1").arg(device_name())) == 0)
        return true;
#endif
    // next: try SCSI

    // scsi START STOP UNIT
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 immed : 1;
        quint8 reserv1 : 7;
        quint8 reserv2;
        quint8 formatlayerno : 2;
        quint8 reserv3 : 6;
        quint8 start : 1;
        quint8 loej : 1;
        quint8 fl : 1;
        quint8 reserv4 : 1;
        quint8 powercond : 4;
        quint8 control;
    } cdb = {};
#pragma pack(pop)

    cdb.opcode = 0x1b;
    cdb.start = 0;
    cdb.loej = 1;

    return exec_scsi_command(&cdb, sizeof(cdb), nullptr, 0);
}

bool drive_handle::allow_removal()
{
    // scsi impl
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 reserved1;
        quint8 reserved2;
        quint8 reserved3;
        quint8 prevent : 1;
        quint8 persistent : 1;
        quint8 reserved4 : 6;
        quint8 control;
    } cdb = {};
#pragma pack(pop)

    cdb.opcode = 0x1e;

    return exec_scsi_command(&cdb, sizeof(cdb), nullptr, 0);
}

void drive_handle::read_cd_text(toc &toc)
{
    // READ TOC/PMA/ATIP response (format = 0101b)
#pragma pack(push, 1)
    struct cdtext_pack {
        quint8 pack_type;
        quint8 track_no;
        quint8 counter;
        quint8 character_pos : 4;
        quint8 block_no : 3;
        quint8 double_byte : 1;
        char textdata[12];
        quint8 crc0;
        quint8 crc1;
    };
    struct {
        quint8 toclen_msb;
        quint8 toclen_lsb;
        quint8 reserved;
        quint8 reserved2;

        struct cdtext_pack cdtext_descriptors[2048];

        quint16 toclen() { return quint16(quint16(toclen_msb) << 8 | quint16(toclen_lsb)); }
        int descriptor_count()
        {
            return std::min(
                    int((toclen()-2) / sizeof(cdtext_descriptors[0])),
                    int(sizeof(cdtext_descriptors)/sizeof(cdtext_descriptors[0])));
        }
    } buf;
#pragma pack(pop)
    std::memset(&buf, 0, sizeof(buf));

    // READ TOC/PMA/ATIP command
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 reserved : 1;
        quint8 msf : 1;
        quint8 reserved1 : 6;
        quint8 format : 4;
        quint8 reserved2 : 4;
        quint8 reserved3;
        quint8 reserved4;
        quint8 reserved5;
        quint8 track_session_no;
        quint8 allocation_length_msb;
        quint8 allocation_length_lsb;
        quint8 control;
    } cmd;
#pragma pack(pop)

    std::memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = 0x43;
    cmd.msf = 1;
    cmd.format = 5;
    cmd.track_session_no = 1;
    cmd.allocation_length_msb = quint8(sizeof(buf) >> 8);
    cmd.allocation_length_lsb = quint8(sizeof(buf) & 0xff);

    if (!exec_scsi_command(&cmd, sizeof(cmd), (quint8*)&buf, sizeof(buf)))
    {
        qWarning() << "Could not read CD-TEXT: " << m_lastErr;
        return;
    }

    // first of all: copy from reponse buffer to vector for further massaging
    std::vector<cdtext_pack> packv;
    packv.reserve(buf.descriptor_count());
    for (int i = 0; i < buf.descriptor_count(); ++i)
    {
        //FIXME: skip anything by block 0
        if (buf.cdtext_descriptors[i].block_no != 0)
            continue;

        //FIXME: skip multibyte stuff
        if (buf.cdtext_descriptors[i].double_byte)
            continue;

        // TODO: verify CRC

        packv.push_back(buf.cdtext_descriptors[i]);
    }

    // ensure everything is ordered by type, then by counter
    // CD drives ususally return it that way, but the MMC spec doesn't actually guarantee it (I believe)
    std::sort(packv.begin(), packv.end(), [](const cdtext_pack &a, const cdtext_pack &b) {
        return a.pack_type < b.pack_type || a.counter < b.counter; });

    // now parse it
    QByteArray textcache;
    textcache.reserve(160); // max size recommended in the spec
    for (const auto &pack: packv)
    {
        if (pack.pack_type != 0x80 /* title */ && pack.pack_type != 0x81 /* artist */)
            continue;

        int track = pack.track_no;
        for (int i = 0; i < 12; ++i)
        {
            if (pack.textdata[i])
            {
                textcache.push_back(pack.textdata[i]);
            }
            else if (textcache.size())
            {
                if (track == 0)
                {
                    if (pack.pack_type == 0x80 /* title */)
                        toc.title = QString::fromLatin1(textcache);
                    if (pack.pack_type == 0x81 /* artist */)
                        toc.artist = QString::fromLatin1(textcache);
                }
                else
                {
                    int toci = toc.index_for_trackno(track);
                    if (toci >= 0)
                    {
                        if (pack.pack_type == 0x80 /* title */)
                            toc.tracks[toci].title = QString::fromLatin1(textcache);
                        if (pack.pack_type == 0x81 /* artist */)
                            toc.tracks[toci].artist = QString::fromLatin1(textcache);
                    }
                }

                track++;
                textcache.clear();
            }
        }
    }
}

toc drive_handle::get_toc()
{
    //==== Read TOC from SCSI

    // READ TOC/PMA/ATIP response (format = 0010b)
#pragma pack(push, 1)
    struct {
        quint8 toclen_msb;
        quint8 toclen_lsb;
        quint8 first_session;
        quint8 last_session;

        struct {
            quint8 session_number;
            quint8 control : 4;
            quint8 adr : 4;
            quint8 tno;
            quint8 point;
            quint8 min;
            quint8 sec;
            quint8 frame;
            quint8 zero;
            quint8 pmin;
            quint8 psec;
            quint8 pframe;
        } track_descriptors[512]; // a CD can contain 99 audio tracks max, this should be plenty enough

        quint16 toclen() { return quint16(quint16(toclen_msb) << 8 | quint16(toclen_lsb)); }
        int descriptor_count()
        {
            return std::min(
                    int((toclen()-2) / sizeof(track_descriptors[0])),
                    int(sizeof(track_descriptors)/sizeof(track_descriptors[0])));
        }
    } buf;
#pragma pack(pop)
    std::memset(&buf, 0, sizeof(buf));

    // READ TOC/PMA/ATIP command
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 reserved : 1;
        quint8 msf : 1;
        quint8 reserved1 : 6;
        quint8 format : 4;
        quint8 reserved2 : 4;
        quint8 reserved3;
        quint8 reserved4;
        quint8 reserved5;
        quint8 track_session_no;
        quint8 allocation_length_msb;
        quint8 allocation_length_lsb;
        quint8 control;
    } cmd;
#pragma pack(pop)

    std::memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = 0x43;
    cmd.msf = 1;
    cmd.format = 2;
    cmd.track_session_no = 1;
    cmd.allocation_length_msb = quint8(sizeof(buf) >> 8);
    cmd.allocation_length_lsb = quint8(sizeof(buf) & 0xff);

    if (!exec_scsi_command(&cmd, sizeof(cmd), (quint8*)&buf, sizeof(buf)))
        return toc();

    //==== build track list from SCSI response
    toc retval;

    for (int i = 0; i < buf.descriptor_count(); ++i)
    {
        const auto &desc = buf.track_descriptors[i];

        toc_track t;
        t.session = desc.session_number;
        t.index = desc.point;
        t.start = block_addr::from_msf(desc.pmin, desc.psec, desc.pframe);
        t.control = desc.control;
        t.adr = desc.adr;

        retval.tracks.push_back(t);

        //qWarning() << "found track: session=" << desc.session_number << "point=" << t.index << "start=" << t.start.to_display() << "adr=" << t.adr << "ctrl=" << t.control;
    }

    // remove everything but tracks and lead-out
    retval.tracks.erase(
                std::remove_if(retval.tracks.begin(), retval.tracks.end(), [](const toc_track &a)  { return a.index > 99 && a.index != 0xa2; }),
                retval.tracks.end());

    // sort by starting block size
    std::sort(retval.tracks.begin(), retval.tracks.end(), [](const toc_track &a, const toc_track &b) { return a.start < b.start; });

    // fill in lengths
    for (auto i = retval.tracks.size()-1; i > 0; --i)
    {
        retval.tracks[i-1].length = retval.tracks[i].start - retval.tracks[i-1].start;
    }

    // remove lead-out entries
    retval.tracks.erase(std::remove_if(retval.tracks.begin(), retval.tracks.end(), [](const toc_track &a) { return a.index > 99; }), retval.tracks.end());

    //=== find media catalog number
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 reserved : 1;
        quint8 msf : 1;
        quint8 reserved2 : 6;
        quint8 reserved3 : 6;
        quint8 subq : 1;
        quint8 reserved4 : 1;
        quint8 subchannel_parameter_list;
        quint8 reserved5[2];
        quint8 track_no;
        quint8 allocation_length_msb;
        quint8 allocation_length_lsb;
        quint8 control;
    } subcmd;
    struct {
        quint8 reserved;
        quint8 audio_status;
        quint8 subch_data_len_msb;
        quint8 subch_data_len_lsb;
        union {
            struct {
                quint8 subchannel_data_format_code;
                quint8 reserved[3];
                quint8 reserved2 : 7;
                quint8 mcval : 1;
                char mcn[14]; // supposedly always zero-terminated by the drive
                quint8 aframe;
            } mcn;
            struct {
                quint8 subchannel_data_format_code;
                quint8 control : 4;
                quint8 adr : 4;
                quint8 trackno;
                quint8 reserved;
                quint8 reserved2 : 7;
                quint8 tcval : 1;
                char isrc[13]; // supposedly always zero-terminated by the drive
                quint8 aframe;
                quint8 reserved3;
            } isrc;
        };
    } subbuf;
#pragma pack(pop)
    std::memset(&subcmd, 0, sizeof(subcmd));
    std::memset(&subbuf, 0, sizeof(subbuf));

    subcmd.opcode = 0x42;
    subcmd.subq = 1;
    subcmd.allocation_length_msb = quint8(quint16(sizeof(subbuf)) >> 8);
    subcmd.allocation_length_lsb = quint8(quint16(sizeof(subbuf)) & 0xff);
    subcmd.subchannel_parameter_list = 2;
    if (exec_scsi_command(&subcmd, sizeof(subcmd), (quint8*)&subbuf, sizeof(subbuf)))
    {
        if (subbuf.mcn.mcval)
        {
            // I wouldn't trust the drive to actually zero terminate the string
            subbuf.mcn.mcn[13] = 0;
            retval.catalog = QString::fromLatin1(subbuf.mcn.mcn);
            qWarning() << "Found MCN:" << retval.catalog;
        }
    }
    else
    {
        qWarning() << "(NON-FATAL) failed to read media catalog number:" << last_error();
    }

    //=== find ISRC for tracks
    for (toc_track &track : retval.tracks)
    {
        std::memset(&subcmd, 0, sizeof(subcmd));
        std::memset(&subbuf, 0, sizeof(subbuf));

        subcmd.opcode = 0x42;
        subcmd.subq = 1;
        subcmd.allocation_length_msb = quint8(quint16(sizeof(subbuf)) >> 8);
        subcmd.allocation_length_lsb = quint8(quint16(sizeof(subbuf)) & 0xff);
        subcmd.subchannel_parameter_list = 3;
        subcmd.track_no = quint8(track.index);

        if (exec_scsi_command(&subcmd, sizeof(subcmd), (quint8*)&subbuf, sizeof(subbuf)))
        {
            if (subbuf.isrc.tcval)
            {
                // I wouldn't trust the drive to actually zero terminate the string
                subbuf.isrc.isrc[12] = 0;
                track.isrc = QString::fromLatin1(subbuf.isrc.isrc);
                qWarning() << "Found ISRC:" << track.isrc;
            }
        }
        else
        {
            qWarning() << "(NON-FATAL) failed to read ISRC for track" << track.index << ":" << last_error();
        }
    }

    //=== search CD-TEXT
    read_cd_text(retval);

    return retval;
}

bool drive_handle::read(void *buffer, block_addr start, block_addr_delta length)
{
    // READ CD command
#pragma pack(push, 1)
    struct {
        quint8 opcode;
        quint8 obsolete : 1;
        quint8 dap : 1;
        quint8 sectorType : 3;
        quint8 reserved : 3;
        quint8 startMSB;
        quint8 start2MSB;
        quint8 start2LSB;
        quint8 startLSB;
        quint8 lengthMSB;
        quint8 length2MSB;
        quint8 lengthLSB;
        quint8 reserved2 : 1;
        quint8 c2ErrorInfo : 2;
        quint8 edcEcc : 1;
        quint8 userData : 1;
        quint8 headerCodes : 2;
        quint8 sync : 1;
        quint8 subChannelSelectionBits : 3;
        quint8 reserved3 : 5;
        quint8 control;
    } cmd;
#pragma pack(pop)
    std::memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = 0xbe;
    cmd.sectorType = 1;
    cmd.startMSB = quint8(quint32(start.block) >> 24);
    cmd.start2MSB = quint8((quint32(start.block) & 0x00ff0000) >> 16);
    cmd.start2LSB = quint8((quint32(start.block) & 0x0000ff00) >> 8);
    cmd.startLSB = quint8((quint32(start.block) & 0x000000ff));
    cmd.lengthMSB = quint8((quint32(length.delta_blocks) & 0x00ff0000) >> 16);
    cmd.length2MSB = quint8((quint32(length.delta_blocks) & 0x0000ff00) >> 8);
    cmd.lengthLSB = quint8((quint32(length.delta_blocks) & 0x000000ff));
    cmd.userData = 1;

    return exec_scsi_command(&cmd, sizeof(cmd), (quint8*)buffer, 2352*length.delta_blocks);
}

bool drive_handle::ok() const
{
#if defined(Q_OS_LINUX)
    return m_fd >= 0;
#elif defined(Q_OS_WIN32)
    return m_handle != 0 && m_handle != INVALID_HANDLE_VALUE;
#else
#error Not implemented for your OS
#endif
}

QStringList drive_handle::list_drives()
{
    QStringList retval;

#if defined(Q_OS_LINUX)
    retval << QStringLiteral("/dev/cdrom");

    for (int i = 0; i < 20 /* FIXME */; ++i)
    {
        auto fn = QStringLiteral("/dev/sr%1").arg(i);
        if (QFileInfo(fn).exists())
        {
            retval << fn;
        }
    }

    return retval;
#elif defined(Q_OS_WIN32)
    for (wchar_t i = L'A'; i <= L'Z'; ++i)
    {
        wchar_t drive[] = { '\\', '\\', '.', '\\', i, ':', 0 };
        wchar_t root[] = { i, ':', '\\', 0 };
        if (GetDriveType(root) == DRIVE_CDROM)
        {
            retval << QString::fromWCharArray(drive);
        }
    }
#else
#error Not implemented for your OS
#endif
    return retval;
}


} // namespace cdda
