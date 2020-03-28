#include "fileutil.h"

#include "libcdda/os_util.h"

#include <QFile>
#include <QDir>

#ifdef Q_OS_WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#endif

QIODevice *FileUtil::createFileExclusive(const QString &filename, QString *error)
{
#if defined(Q_OS_UNIX)
    QByteArray filename8bit = QFile::encodeName(filename);

    error->clear();

    int fd = open(filename8bit.constData(), O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd >= 0)
    {
        QFile *retval = new QFile();
        if (retval->open(fd, QIODevice::WriteOnly, QFile::AutoCloseHandle))
            return retval;

        *error = retval->errorString();
        delete retval;
        return nullptr;
    }

    if (errno == EEXIST)
        return nullptr;

    *error = cdda::os_error_to_str(errno);
    return nullptr;
#elif defined(Q_OS_WIN32)
    QString nativeFn = QDir::toNativeSeparators(filename);
    HANDLE h = CreateFile(LPCWSTR(nativeFn.utf16()),
                          GENERIC_WRITE, 0, nullptr,
                          CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                          nullptr);
    if (h != 0 && h != INVALID_HANDLE_VALUE)
    {
        // QFile won't take our handle - so close it and open it again
        CloseHandle(h);
        QFile *retval = new QFile(filename);
        if (retval->open(QIODevice::WriteOnly))
            return retval;

        *error = retval->errorString();
        delete retval;
        return nullptr;
    }

    if (GetLastError() == ERROR_FILE_EXISTS)
        return nullptr;

    *error = cdda::os_error_to_str(GetLastError());
    return nullptr;
#else
#error createFileExclusive() not implemented for your OS
#endif
}

QString FileUtil::sanitizeFilename(QString filename)
{
    // replace all disallowed characters with underscores
    for (int i = 0; i < filename.size(); ++i) {
        QChar c = filename[i];
        if (c == QLatin1Char('<') || c == QLatin1Char('>') || c == QLatin1Char(':')
                || c == QLatin1Char('"') || c == QLatin1Char('/') || c == QLatin1Char('\\')
                || c == QLatin1Char('|') || c == QLatin1Char('?') || c == QLatin1Char('*')
                || c < 32)
            filename[i] = QLatin1Char('_');
    }

    // fix reserved names by prepending an underscore
    QString basename = filename.left(filename.lastIndexOf(QLatin1Char('.'))).toUpper();
    if (basename == QLatin1String("CON") || basename == QLatin1String("PRN")
            || basename == QLatin1String("AUX") || basename == QLatin1String("NUL")
            || basename == QLatin1String("COM1") || basename == QLatin1String("COM2")
            || basename == QLatin1String("COM3") || basename == QLatin1String("COM4")
            || basename == QLatin1String("COM5") || basename == QLatin1String("COM6")
            || basename == QLatin1String("COM7") || basename == QLatin1String("COM8")
            || basename == QLatin1String("COM9") || basename == QLatin1String("LPT1")
            || basename == QLatin1String("LPT2") || basename == QLatin1String("LPT3")
            || basename == QLatin1String("LPT4") || basename == QLatin1String("LPT5")
            || basename == QLatin1String("LPT6") || basename == QLatin1String("LPT7")
            || basename == QLatin1String("LPT8") || basename == QLatin1String("LPT9")) {
        filename = QStringLiteral("_%1%2").arg(basename, filename.right(filename.size() - basename.size()));
    }

    return filename;
}
