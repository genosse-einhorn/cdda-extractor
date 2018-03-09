#include "releasefinder.h"

#include "urldownloader.h"

#include <QXmlStreamReader>
#include <vector>
#include <stack>
#include <QDebug>

namespace MusicBrainz {

ReleaseFinder::ReleaseFinder(QObject *parent) : QObject(parent)
{

}

void ReleaseFinder::startSearch(const QString &discid, const QString &mcn)
{
    m_discid = discid;
    m_mcn = mcn;

    QString url = QStringLiteral("https://musicbrainz.org/ws/2/discid/%1?inc=recordings+isrcs+labels").arg(discid);
    UrlDownloader *downloader = new UrlDownloader(url, this);
    connect(downloader, &UrlDownloader::downloaded, this, &ReleaseFinder::discidDataArrived);
    connect(downloader, &UrlDownloader::downloaded, downloader, &UrlDownloader::deleteLater);
}

void ReleaseFinder::startMetadataSearch()
{
    QString url = QStringLiteral("https://musicbrainz.org/ws/2/release/%1"
        "?inc=recordings+artist-credits+recording-rels+work-rels+recording-level-rels+work-level-rels+discids+artist-rels").arg(m_release);
    UrlDownloader *downloader = new UrlDownloader(url, this);
    connect(downloader, &UrlDownloader::downloaded, this, &ReleaseFinder::releaseDataArrived);
    connect(downloader, &UrlDownloader::downloaded, downloader, &UrlDownloader::deleteLater);
}

void ReleaseFinder::startCoverDownload()
{
    QString url = QStringLiteral("https://coverartarchive.org/release/%1/front-500").arg(m_release);
    UrlDownloader *downloader = new UrlDownloader(url, this);
    connect(downloader, &UrlDownloader::downloaded, this, &ReleaseFinder::coverArrived);
    connect(downloader, &UrlDownloader::downloaded, downloader, &UrlDownloader::deleteLater);
}

void ReleaseFinder::discidDataArrived(const QByteArray &data)
{
    struct Release
    {
        QString id;
        QStringList catalogNrs;
    };
    std::vector<Release> availableReleases;

    QXmlStreamReader xml(data);

    while (!xml.atEnd())
    {
        if (xml.readNextStartElement())
        {
            if (xml.name() == QStringLiteral("release"))
            {
                Release r;
                r.id = xml.attributes().value(QStringLiteral("id")).toString();
                if (!r.id.size())
                {
                    qWarning() << "WTF: <release> without id attribute?";
                    continue;
                }

                while (xml.readNextStartElement())
                {
                    if (xml.name() == QStringLiteral("barcode"))
                    {
                        r.catalogNrs << xml.readElementText();
                    }
                    // TODO: else if label-info-list
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }

                availableReleases.push_back(r);
            }
        }
    }

    if (xml.hasError())
    {
        qWarning() << "XML parse error:" << xml.errorString();
    }

    if (availableReleases.size())
    {
        QStringRef mcnWithoutLeadingZeroes(&m_mcn);
        while (mcnWithoutLeadingZeroes.startsWith(QLatin1Char('0')))
            mcnWithoutLeadingZeroes = mcnWithoutLeadingZeroes.right(mcnWithoutLeadingZeroes.length()-1);

        // check if we have a release with the right catalog nr
        for (const Release &r : availableReleases)
        {
            if (r.catalogNrs.contains(m_mcn) || r.catalogNrs.contains(mcnWithoutLeadingZeroes.toString()))
            {
                m_release = r.id;
                startMetadataSearch();
                return;
            }
        }

        // otherwise just return the first
        m_release = availableReleases[0].id;
        startMetadataSearch();
    }
    else
    {
        emit noMetadataFound();
    }
}

void ReleaseFinder::releaseDataArrived(const QByteArray &data)
{
    QXmlStreamReader xml(data);

    m_metadata = ReleaseMetadata();

    while (!xml.atEnd())
    {
        if (xml.readNextStartElement())
        {
            if (xml.name() == QStringLiteral("release"))
            {
                if (xml.attributes().value(QLatin1Literal("id")) != m_release)
                    qWarning() << "WTF: got different release id";

                parseRelease(&xml);
            }
        }
    }

    if (xml.hasError())
    {
        qWarning() << "XML parse error:" << xml.errorString();
    }

    startCoverDownload();
}

void ReleaseFinder::coverArrived(const QByteArray &data)
{
    if (data.size())
    {
        m_metadata.cover = QImage::fromData(data);
    }
    else
    {
        qWarning() << "No cover found";
    }

    emit metadataFound(m_metadata);
}

void ReleaseFinder::parseRelease(QXmlStreamReader *xml)
{
    Q_ASSERT(xml->name() == QStringLiteral("release"));
    m_metadata.releaseId = xml->attributes().value(QStringLiteral("id")).toString();

    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("title"))
        {
            m_metadata.title = xml->readElementText();
        }
        else if (xml->name() == QStringLiteral("date"))
        {
            QString date = xml->readElementText();
            int pos = 0;
            while (pos < date.size() && date[pos] >= QLatin1Char('0') && date[pos] <= QLatin1Char('9'))
                pos++;
            m_metadata.year = date.left(pos);
        }
        else if (xml->name() == QStringLiteral("artist-credit"))
        {
            m_metadata.artist = parseArtistCredits(xml);
        }
        else if (xml->name() == QStringLiteral("medium-list"))
        {
            while (xml->readNextStartElement())
            {
                if (xml->name() == QStringLiteral("medium"))
                {
                    parseMedium(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
        }
        else
        {
            xml->skipCurrentElement();
        }
    }
}

QString ReleaseFinder::parseArtistCredits(QXmlStreamReader *xml)
{
    QStringList names;

    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("name-credit"))
        {
            while (xml->readNextStartElement())
            {
                if (xml->name() == QStringLiteral("artist"))
                {
                    while (xml->readNextStartElement())
                    {
                        if (xml->name() == QStringLiteral("name"))
                        {
                            names << xml->readElementText();
                        }
                        else
                        {
                            xml->skipCurrentElement();
                        }
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
        }
        else
        {
            xml->skipCurrentElement();
        }
    }

    return names.join(QStringLiteral(", "));
}

void ReleaseFinder::parseMedium(QXmlStreamReader *xml)
{
    // ugly: MusicBrainz gives us a list of media, one of these is the CD currently inserted
    // but we can only check that when we at some point stumble upon a matching disc id
    bool isCorrect = false;
    QString discNo;
    std::vector<TrackMetadata> trackMeta;

    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("position"))
        {
            discNo = xml->readElementText();
        }
        else if (xml->name() == QStringLiteral("disc-list"))
        {
            while (xml->readNextStartElement())
            {
                if (xml->name() == QStringLiteral("disc"))
                {
                    if (xml->attributes().value(QStringLiteral("id")) == m_discid)
                        isCorrect = true;
                }

                xml->skipCurrentElement();
            }
        }
        else if (xml->name() == QStringLiteral("track-list"))
        {
            while (xml->readNextStartElement())
            {
                if (xml->name() == QStringLiteral("track"))
                {
                    trackMeta.push_back(parseTrack(xml));
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
        }
        else
        {
            xml->skipCurrentElement();
        }
    }

    if (isCorrect)
    {
        m_metadata.discNo = discNo;
        m_metadata.tracks = trackMeta;
    }
}

TrackMetadata ReleaseFinder::parseTrack(QXmlStreamReader *xml)
{
    TrackMetadata metadata;
    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("number"))
        {
            metadata.trackno = xml->readElementText().toInt();
        }
        else if (xml->name() == QStringLiteral("recording"))
        {
            auto r = parseRecording(xml);
            metadata.artist = r.artist;
            metadata.title = r.title;
            metadata.composer = r.composer;
        }
        else
        {
            xml->skipCurrentElement();
        }
    }

    return metadata;
}

TrackMetadata ReleaseFinder::parseRecording(QXmlStreamReader *xml)
{
    TrackMetadata meta;

    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("title"))
        {
            meta.title = xml->readElementText();
        }
        else if (xml->name() == QStringLiteral("artist-credit"))
        {
            meta.artist = parseArtistCredits(xml);
        }
        else if (xml->name() == QStringLiteral("relation-list")
                 && xml->attributes().value(QStringLiteral("target-type")) == QStringLiteral("work"))
        {
            while (xml->readNextStartElement())
            {
                if (xml->name() == QStringLiteral("relation"))
                {
                    while (xml->readNextStartElement())
                    {
                        if (xml->name() == QStringLiteral("work"))
                        {
                            meta.composer = parseComposerFromWork(xml);
                        }
                        else
                        {
                            xml->skipCurrentElement();
                        }
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
        }
        else
        {
            xml->skipCurrentElement();
        }
    }

    return meta;
}

QString ReleaseFinder::parseComposerFromWork(QXmlStreamReader *xml)
{
    QStringList composers;

    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("relation-list")
                && xml->attributes().value(QStringLiteral("target-type")) == QStringLiteral("artist"))
        {
            while (xml->readNextStartElement())
            {
                if (xml->name() == QStringLiteral("relation")
                        && xml->attributes().value(QStringLiteral("type")) == QStringLiteral("composer"))
                {
                    while (xml->readNextStartElement())
                    {
                        if (xml->name() == QStringLiteral("artist"))
                        {
                            while (xml->readNextStartElement())
                            {
                                if (xml->name() == QStringLiteral("name"))
                                {
                                    composers << xml->readElementText();
                                }
                                else
                                {
                                    xml->skipCurrentElement();
                                }
                            }
                        }
                        else
                        {
                            xml->skipCurrentElement();
                        }
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
        }
        else
        {
            xml->skipCurrentElement();
        }
    }

    return composers.join(QStringLiteral(", "));
}

} // namespace MusicBrainz
