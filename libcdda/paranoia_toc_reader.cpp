#include "paranoia_toc_reader.h"

#include <QRegExp>
#include <QDebug>

namespace cdda {

paranoia_toc_reader::paranoia_toc_reader(QObject *parent) : QObject(parent)
{
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &paranoia_toc_reader::processFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &paranoia_toc_reader::processErrord);
    connect(&m_process, &QProcess::readyRead, this, &paranoia_toc_reader::readAvailable);
}

void paranoia_toc_reader::start()
{
    if (m_process.state() != QProcess::NotRunning)
        return;

    m_process.setProgram(QStringLiteral("cdparanoia"));
    m_process.setArguments(QStringList() << QStringLiteral("-Q"));

    m_process.setStandardInputFile(QProcess::nullDevice());
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    m_tracks.clear();
    m_state = WAITING_FOR_START;
    m_errorLineBuf.clear();
    m_process.start();
}

void paranoia_toc_reader::readAvailable()
{
    while (m_process.canReadLine())
    {
        char buf[1024] = {0};
        m_process.readLine(buf, sizeof(buf));

        QString line = QString::fromUtf8(buf).trimmed();

        // we will save all messages since they are potential errors to show the user
        m_errorLineBuf << line;

        switch (m_state)
        {
        case WAITING_FOR_START:
            // start of list is marked by line full of "==="
            if (QRegExp(QStringLiteral("=+")).exactMatch(line))
            {
                m_state = IN_LIST;
            }

            break;
        case IN_LIST:
        {
            QRegExp tocRe(QStringLiteral("\\s*(\\d+)\\.\\s+(\\d+)\\s+\\[\\d+:\\d+.\\d+\\]\\s+(\\d+)\\s+\\[\\d+:\\d+.\\d+\\].*"));
            if (tocRe.exactMatch(line))
            {
                toc_track track;
                track.index = tocRe.cap(1).toInt();
                track.start = block_addr::from_lba(tocRe.cap(3).toInt());
                track.length = block_addr_delta::from_lba(tocRe.cap(2).toInt());
                m_tracks.push_back(track);
            }

            if (QRegExp(QStringLiteral("TOTAL\\s+\\d+\\s*\\[\\d+:\\d+.\\d+\\].*")).exactMatch(line))
                m_state = AFTER_END;
        }
            break;
        case AFTER_END:
            break;
        }
    }
}

void paranoia_toc_reader::processErrord()
{
    emit error(m_process.errorString());
}

void paranoia_toc_reader::processFinished()
{
    if (m_process.exitCode() != 0 || m_tracks.size() == 0)
    {
        emit error(m_errorLineBuf.join(QStringLiteral("\n")).trimmed());
        return;
    }

    emit success(m_tracks);
}

} // namespace cdda
