#ifndef TASKRUNNER_SYNCURLDOWNLOAD_H
#define TASKRUNNER_SYNCURLDOWNLOAD_H

#include "taskrunner.h"

namespace TaskRunner {

QByteArray downloadUrl(const QString &url, const CancelToken &cancelToken);

} // namespace TaskRunner

#endif // TASKRUNNER_SYNCURLDOWNLOAD_H
