#ifndef OS_UTIL_H
#define OS_UTIL_H

#include <QString>

#ifdef Q_OS_UNIX
#   include <errno.h>
#endif
#ifdef Q_OS_WIN32
#   include <windows.h>
#endif

namespace cdda {

QString os_error_to_str(
#ifdef Q_OS_WIN32
        DWORD
#else
        int
#endif
        error);

} // namespace cdda

#endif // OS_UTIL_H
