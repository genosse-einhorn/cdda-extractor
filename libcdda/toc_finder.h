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

cdda::toc find_toc(QString *out_device, QStringList *out_log, enum result_sense *out_sense, const TaskRunner::CancelToken &cancelToken);

} // namespace cdda

#endif // TOC_FINDER_H
