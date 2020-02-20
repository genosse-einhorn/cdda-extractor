#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QString>

class QIODevice;

namespace FileUtil {
    QIODevice *createFileExclusive(const QString &filename, QString *error);

    QString sanitizeFilename(QString filename);
} // namespace FileUtil

#endif // FILEUTIL_H
