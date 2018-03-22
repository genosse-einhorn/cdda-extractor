#ifndef WIN32ICONLOADER_H
#define WIN32ICONLOADER_H

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <QIcon>
#include <QtWin>
#include <shellapi.h>

namespace IconLoader {

QIcon fromFile(const QString &file, int index);
QIcon fromShellStock(SHSTOCKICONID shstockiconid);

} // IconLoader

#endif // WIN32

#endif // WIN32ICONLOADER_H
