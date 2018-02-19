#ifndef RELEASEFINDER_H
#define RELEASEFINDER_H

#include <QObject>
#include <QImage>
#include <vector>

class QXmlStreamReader;

namespace MusicBrainz {

struct TrackMetadata {
    int trackno;
    QString title;
    QString artist;
    QString composer;
};

struct ReleaseMetadata {
    QString releaseId;
    QString title;
    QString artist;
    QString composer;
    QString year;
    QString discNo;
    QImage cover;
    std::vector<TrackMetadata> tracks;
};

class ReleaseFinder : public QObject
{
    Q_OBJECT
public:
    explicit ReleaseFinder(QObject *parent = nullptr);

signals:
    void metadataFound(const ReleaseMetadata &meta);
    void noMetadataFound();

public slots:
    void startSearch(const QString &discid, const QString &mcn);

private slots:
    void startMetadataSearch();
    void startCoverDownload();

    void discidDataArrived(const QByteArray &data);
    void releaseDataArrived(const QByteArray &data);
    void coverArrived(const QByteArray &data);

private:
    QString m_discid;
    QString m_mcn;
    QString m_release;
    ReleaseMetadata m_metadata;

    void parseRelease(QXmlStreamReader *xml);
    QString parseArtistCredits(QXmlStreamReader *xml);
    void parseMedium(QXmlStreamReader *xml);
    TrackMetadata parseTrack(QXmlStreamReader *xml);
    TrackMetadata parseRecording(QXmlStreamReader *xml);
    QString parseComposerFromWork(QXmlStreamReader *xml);
};

} // namespace MusicBrainz

#endif // RELEASEFINDER_H
