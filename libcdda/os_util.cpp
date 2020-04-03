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

#include "os_util.h"

#ifndef Q_OS_WIN32
#   include <string.h>
#endif

namespace cdda {

#ifdef Q_OS_WIN32

QString os_error_to_str(DWORD err)
{
    wchar_t *str = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, err, 0, (wchar_t *)&str, 0, nullptr);
    QString result = str ? QString::fromWCharArray(str) : QStringLiteral("Unknown Error %1").arg(err);
    LocalFree(str);
    return result;
}

#else

QString os_error_to_str(int error)
{
    char buf[1024] = {0};

    // bizarro GNU interface
    // we'd really like to get rid of it, but Qt headers require _GNU_SOURCE
    char *t = buf;
#ifdef _GNU_SOURCE
    t = strerror_r(error, buf, sizeof(buf));
#else
    strerror_r(error, buf, sizeof(buf));
#endif
    return QStringLiteral("Error %1: %2").arg(error).arg(QString::fromUtf8(t));
}

#endif

} // namespace cdda
