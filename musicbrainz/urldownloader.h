#ifndef URLDOWNLOADER_H
#define URLDOWNLOADER_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace MusicBrainz {

class UrlDownloader : public QObject
{
    Q_OBJECT
public:
    explicit UrlDownloader(const QString &url, QObject *parent = nullptr);
    QByteArray data() const { return m_data; }

signals:
    void downloaded(const QByteArray &data);

private slots:
#ifndef Q_OS_WIN32
    void fileDownloaded(QNetworkReply *reply);
#endif

private:
#ifndef Q_OS_WIN32
    QNetworkAccessManager *m_nam;
#endif
    QByteArray m_data;
};

} // namespace MusicBrainz

#endif // URLDOWNLOADER_H
