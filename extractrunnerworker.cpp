#include "extractrunnerworker.h"

#include <QApplication>
#include <QFile>
#include <QDir>
#ifdef Q_OS_UNIX
#   include <errno.h>
#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#endif
#include <memory>
#include <QThread>

#include "libcdda/os_util.h"
#include "libcdda/drive_handle.h"

#include "encoder/wavencoder.h"

namespace {

QIODevice *createFileExclusive(const QString &filename, QString *error)
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
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
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

    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return nullptr;

    *error = cdda::os_error_to_str(GetLastError());
    return nullptr;
#else
#error openFileExclusive() not implemented for your OS
#endif
}

} // anonymous namespace

ExtractRunnerWorker::ExtractRunnerWorker(QObject *parent) : QObject(parent)
{

}

void ExtractRunnerWorker::openDevice(const QString &device)
{
    m_handle = cdda::drive_handle::open(device);
    if (!m_handle)
    {
        emit failed(tr("Could not open %1: %2").arg(device).arg(m_handle.last_error()));
    }
}

void ExtractRunnerWorker::beginExtract(const QString &directory, const QString &basename,
                                       const QString &format, cdda::block_addr start,
                                       cdda::block_addr_delta length, const cdda::track_metadata &metadata)
{
    m_cancelRequested = false;

    // create directories
    if (!QDir(directory).mkpath(QStringLiteral(".")))
    {
        emit failed(tr("Could not create directory %1").arg(directory));
        return;
    }

    // create file without overwriting files already there
    QString filename = QStringLiteral("%1/%2.%3").arg(directory, basename, format);
    QString error;
    std::unique_ptr<QIODevice> device(createFileExclusive(filename, &error));
    unsigned c = 0;
    while (!device.get() && !error.size())
    {
        // file exists - try next one
        ++c;
        filename = QStringLiteral("%1/%2 (%4).%3").arg(directory, basename, format).arg(c);
        device.reset(createFileExclusive(filename, &error));
    }

    if (!device.get())
    {
        emit failed(tr("Could not create file %1: %2").arg(filename, error));
        return;
    }

    QCoreApplication::processEvents();
    if (m_cancelRequested)
    {
        emit failed(tr("Canceled by user."));
        return;
    }

    // create encoder instance
    std::unique_ptr<Encoder::AbstractEncoder> encoder;

    encoder.reset(new Encoder::WavEncoder());

    // initialize encoder
    if (!encoder->initialize(device.get(), length.delta_blocks*588, metadata))
    {
        emit failed(tr("Failed to initialize encoder: %1").arg(encoder->errorText()));
        return;
    }

    // loop for reading and encoding audio data
    const int STEP_NO = 20;
    const cdda::block_addr_delta STEP_BLOCKS = cdda::block_addr_delta::from_lba(STEP_NO);
    for (auto i = start; i < start + length; i += STEP_BLOCKS)
    {
        qint16 buf[STEP_NO*2352/2] = {};
        cdda::block_addr_delta s = std::min(STEP_BLOCKS, start + length - i);

        // read cd audio
        if (!m_handle.read(buf, i, s))
        {
            emit failed(tr("Failed to read from CD :51").arg(m_handle.last_error()));
            return;
        }

        // encode it
        if (!encoder->feed(buf, 588*s.delta_blocks))
        {
            emit failed(tr("Failed while encoding audio: %1").arg(encoder->errorText()));
            return;
        }

        emit progress(s);

        QCoreApplication::processEvents();
        if (m_cancelRequested)
        {
            emit failed(tr("Canceled by user."));
            return;
        }
    }

    // finalize encoding
    if (!encoder->finish())
    {
        emit failed(tr("Failed to finalize encoding: %1").arg(encoder->errorText()));
        return;
    }

    emit finished();
}

void ExtractRunnerWorker::cancel()
{
    m_cancelRequested = true;
}
