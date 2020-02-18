#include "extractor.h"

#include "tasklib/taskrunner.h"
#include "libcdda/drive_handle.h"
#include "libcdda/os_util.h"
#include "paranoia/paranoia.h"
#include "encoder/flacencoder.h"
#include "encoder/lameencoder.h"
#include "encoder/wavencoder.h"

#include <QFile>
#include <QDir>

#ifdef Q_OS_WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#endif

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

struct ReaderBase
{
    const TaskRunner::CancelToken &m_cancelToken;

    ReaderBase(const TaskRunner::CancelToken &cancelToken) : m_cancelToken(cancelToken) {}
    virtual ~ReaderBase() {}
    virtual bool eof() = 0;
    virtual bool read(qint16 **pBuffer, cdda::block_addr_delta *pBuflen) = 0;
    virtual QString errorMessage() = 0;
};

class RawReader : public ReaderBase
{
private:
    cdda::drive_handle &m_handle;
    cdda::block_addr m_pos;
    cdda::block_addr_delta m_remaining;
    qint16 m_buffer[588*2*20]; // FIXME: why 20 blocks?

public:
    RawReader(cdda::drive_handle &handle, cdda::block_addr start, cdda::block_addr_delta len, const TaskRunner::CancelToken &cancelToken)
        : ReaderBase(cancelToken), m_handle(handle), m_pos(start), m_remaining(len)
    {}

    bool eof() override { return m_remaining.delta_blocks <= 0; }
    bool read(qint16 **pBuffer, cdda::block_addr_delta *pBuflen) override
    {
        if (m_cancelToken.isCanceled())
            return false;

        auto count = std::min(cdda::block_addr_delta::from_lba(int(sizeof(m_buffer)/2352)), m_remaining);
        if (m_handle.read(m_buffer, m_pos, count))
        {
            *pBuffer = m_buffer;
            *pBuflen = count;
            m_pos += count;
            m_remaining -= count;
            return true;
        }
        else
        {
            *pBuffer = nullptr;
            *pBuflen = cdda::block_addr_delta::from_lba(0);
            return false;
        }
    }
    QString errorMessage() override
    {
        if (m_cancelToken.isCanceled())
        {
            return Extractor::tr("Canceled by user");
        }
        else
        {
            return m_handle.last_error();
        }
    }
};

class ParanoiaReader : public ReaderBase
{
private:
    cdda::drive_handle &m_handle;
    int m_blocksRemaining;
    cdrom_paranoia *m_paranoiaHandle;

    static long paranoiaReadfunc(void *userdata, void *buffer, long start, long len)
    {
        auto self = (ParanoiaReader*)userdata;

        if (self->m_cancelToken.isCanceled())
            return CDDA_ERROR_CANCELED;

        if (self->m_handle.read(buffer, cdda::block_addr::from_lba(int(start)), cdda::block_addr_delta::from_lba(int(len))))
            return len;
        else
            return -1L;
    }

public:
    ParanoiaReader(cdda::drive_handle &handle, cdda::block_addr start, cdda::block_addr_delta len, const TaskRunner::CancelToken &cancelToken)
        : ReaderBase(cancelToken), m_handle(handle)
    {
        m_paranoiaHandle = paranoia_init(&ParanoiaReader::paranoiaReadfunc,
                                         this,
                                         10, // FIXME: Why? It works, but I'd like to know why higher values don't work
                                         start.block, (start+len).block - 1); // FIXME! should use first and last sector of whole disc
        paranoia_modeset(m_paranoiaHandle, PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP);
        paranoia_set_range(m_paranoiaHandle, start.block, (start+len).block - 1);

        m_blocksRemaining = len.delta_blocks;
    }
    ~ParanoiaReader() override
    {
        paranoia_free(m_paranoiaHandle);
    }

    bool eof() override { return m_blocksRemaining <= 0; }
    bool read(qint16 **pBuffer, cdda::block_addr_delta *pBuflen) override
    {
        *pBuffer = paranoia_read(m_paranoiaHandle, nullptr);
        *pBuflen = cdda::block_addr_delta::from_lba(1);
        if (*pBuffer != nullptr)
        {
            m_blocksRemaining -= 1;
            return true;
        }
        else
        {
            return false;
        }
    }
    QString errorMessage() override
    {
        if (m_cancelToken.isCanceled())
        {
            return Extractor::tr("Canceled by user");
        }
        else
        {
            return m_handle.last_error();
        }
    }
};

QString extractTrack(cdda::drive_handle &handle,
                     const Extractor::TrackToExtract &track,
                     const QString &outdir,
                     const QString &format,
                     const TaskRunner::CancelToken &cancelToken,
                     const TaskRunner::ProgressToken &progressToken,
                     Extractor::ReadingMode mode)
{
    // initialize reader
    std::unique_ptr<ReaderBase> reader;

    if (mode == Extractor::READ_PARANOIA)
        reader.reset(new ParanoiaReader(handle, track.start, track.length, cancelToken));
    else
        reader.reset(new RawReader(handle, track.start, track.length, cancelToken));

    // create directories
    if (!QDir(outdir).mkpath(QStringLiteral(".")))
        return Extractor::tr("Could not create directory %1").arg(outdir);


    // create file without overwriting files already there
    QString basename = QStringLiteral("%1 - %2")
            .arg(track.metadata.trackno, 2, 10, QLatin1Char('0'))
            .arg(track.metadata.title.size() ? track.metadata.title : QStringLiteral("Track %1").arg(track.metadata.trackno));

    QString filename = QStringLiteral("%1/%2.%3").arg(outdir, basename, format);

    QString error;
    std::unique_ptr<QIODevice> device(createFileExclusive(filename, &error));
    unsigned c = 0;
    while (!device.get() && !error.size())
    {
        // file exists - try next one
        ++c;
        filename = QStringLiteral("%1/%2 (%4).%3").arg(outdir, basename, format).arg(c);
        device.reset(createFileExclusive(filename, &error));
    }

    if (!device.get())
        return Extractor::tr("Could not create file %1: %2").arg(filename, error);

    // create encoder instance
    std::unique_ptr<Encoder::AbstractEncoder> encoder;

    if (format == QLatin1Literal("flac"))
        encoder.reset(new Encoder::FlacEncoder());
    else if (format == QLatin1Literal("mp3"))
        encoder.reset(new Encoder::LameEncoder());
    else
        encoder.reset(new Encoder::WavEncoder());

    // initialize encoder
    if (!encoder->initialize(device.get(), track.length.delta_blocks*588, track.metadata))
        return Extractor::tr("Failed to initialize encoder: %1").arg(encoder->errorText());

    // now extract data
    cdda::block_addr_delta processed_blocks { 0 };

    // really simple pre-gap detection: chop off silence from the end
    qint64 trailingSilentSamples = 0;

    // loop for reading and encoding audio data
    while (!reader->eof())
    {
        qint16 *buf;
        cdda::block_addr_delta buflen { 0 };
        if (!reader->read(&buf, &buflen))
            return Extractor::tr("Failed to read from CD: %1").arg(reader->errorMessage());

        // pre-gap detection
        qint64 samples = 588*buflen.delta_blocks;
        qint64 silentSamplesThisBuf = 0;
        while (samples > 0 && !buf[2*samples-1] && !buf[2*samples-2])
        {
            samples--;
            silentSamplesThisBuf++;
        }

        if (samples)
        {
            // write buffered silent samples
            while (trailingSilentSamples > 0)
            {
                qint16 silentBuf[200] = {};
                qint64 n = std::min(trailingSilentSamples, qint64(sizeof(silentBuf)/sizeof(silentBuf[0]))/2);
                if (!encoder->feed(silentBuf, n))
                    return Extractor::tr("Failed while encoding audio: %1").arg(encoder->errorText());

                trailingSilentSamples -= n;
            }

            // encode it
            if (!encoder->feed(buf, samples))
                return Extractor::tr("Failed while encoding audio: %1").arg(encoder->errorText());
        }

        trailingSilentSamples += silentSamplesThisBuf;

        processed_blocks += buflen;
        progressToken.reportProgressValueAndText(progressToken.progressValue() + buflen.delta_blocks,
                                                 Extractor::tr("Extracting Track %1 (%2)")
                                                             .arg(track.metadata.trackno)
                                                             .arg(processed_blocks.to_display()));
    }

    // finalize encoding
    if (!encoder->finish())
        return Extractor::tr("Failed to finalize encoding: %1").arg(encoder->errorText());

    return QString();
}

} // anonymous namespace

QFuture<QString> Extractor::extract(const QString &device, const QString &outdir, const QString &format,
                                    const std::vector<Extractor::TrackToExtract> &tracks,
                                    ReadingMode mode)
{
    return TaskRunner::run([=](const TaskRunner::CancelToken &cancelToken, const TaskRunner::ProgressToken &progressToken) {
        // initialize progress reporting
        int totallen = 0;
        for (const auto &track : tracks) {
            totallen += track.length.delta_blocks;
        }

        progressToken.reportProgressRange(0, totallen);
        progressToken.reportProgressValueAndText(1, tr("Initializing..."));

        // open drive
        cdda::drive_handle handle = cdda::drive_handle::open(device);
        if (!handle) {
            return tr("Could not open %1: %2").arg(device).arg(handle.last_error());
        }

        // extract tracks
        for (const auto &track : tracks) {
            QString e = extractTrack(handle, track, outdir, format, cancelToken, progressToken, mode);
            if (e.length())
                return e;
        }

        return QString();
    });
}
