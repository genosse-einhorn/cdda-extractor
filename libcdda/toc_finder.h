#ifndef TOC_FINDER_H
#define TOC_FINDER_H

#include <QObject>
#include <QFuture>

#include "../tasklib/taskrunner.h"
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

cdda::toc find_toc(QString *out_device, QStringList *out_log, const TaskRunner::CancelToken &cancelToken);

QFuture<toc_find_result> find_toc_threaded(void);

} // namespace cdda

#endif // TOC_FINDER_H
