#ifndef TOC_FINDER_H
#define TOC_FINDER_H

#include <QObject>
#include <QFuture>

#include "toc.h"

namespace cdda {

// HACK! keep for tr() support
class toc_finder : public QObject
{
    Q_OBJECT
};

struct toc_find_result {
    QStringList log;
    QString device;
    cdda::toc toc;
};

QFuture<toc_find_result> find_toc(void);

} // namespace cdda

#endif // TOC_FINDER_H
