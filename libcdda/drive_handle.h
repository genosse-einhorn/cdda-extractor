#ifndef CDDA_DRIVE_HANDLE_H
#define CDDA_DRIVE_HANDLE_H

#include <QString>
#include <QStringList>
#include <vector>

#ifdef Q_OS_WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include "toc.h"

namespace cdda {

class drive_handle
{
private:
#ifdef Q_OS_LINUX
    int m_fd { -1 };
#endif
#ifdef Q_OS_WIN32
    HANDLE m_handle;
#endif
    QString m_deviceName;
    QString m_lastErr;
    enum result_sense m_lastSenseEnum { RESULT_SENSE_OK };

    void cleanup();    

public:
    drive_handle();
    drive_handle(drive_handle &&other);
    drive_handle(const drive_handle &other) = delete;
    ~drive_handle();
    drive_handle &operator=(const drive_handle &other) = delete;
    drive_handle &operator=(drive_handle &&other);

    static drive_handle open(const QString &drive);

    // scsi command support
    bool exec_scsi_command(void *cmd, qint64 cmdlen, quint8 *bufp, qint64 buflen);

    // convenience stuff
    bool eject();
    bool close_tray();

    bool prevent_removal();
    bool allow_removal();

    toc get_toc();
    void fill_mcn(toc &toc);
    void fill_track_isrc(toc_track &track);
    void fill_cd_text(toc &toc);

    bool read(void *buffer, block_addr start, block_addr_delta length);


    bool ok() const;
    explicit operator bool() const { return ok(); }
    QString last_error() const { return m_lastErr; }
    enum result_sense last_sense_code() const { return m_lastSenseEnum; }
    QString device_name() const { return m_deviceName; }

    // finding drives
    static QStringList list_drives();
};

} // namespace cdda

#endif // CDDA_DRIVE_HANDLE_H
