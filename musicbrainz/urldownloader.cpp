#include "urldownloader.h"

static const QString USER_AGENT = QStringLiteral("KuemCddaExtract/0.0 (jonas@kuemmerlin.eu)");

/* On Win32, we don't want to use the Qt network functionality since that would require us to ship
 * OpenSSL and push out updates for every OpenSSL CVE. We'll rather spend a little bit of time to
 * use one of the HTTP clients conveniently supplied with the operating system.
 *
 * On Unix, OpenSSL is provided by the OS vendor so we can use Qt.
 */

#ifdef Q_OS_WIN32

#include <windows.h>
#include <wininet.h>

#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include "../libcdda/os_util.h"

static QByteArray downloadWinInet(const QString &url)
{
    HINTERNET inet = InternetOpen(LPCWSTR(USER_AGENT.utf16()),
                                  INTERNET_OPEN_TYPE_PRECONFIG,
                                  nullptr,
                                  nullptr,
                                  0);
    if (!inet)
    {
        qWarning() << "InternetOpen() failed:" << cdda::os_error_to_str(GetLastError());
        return QByteArray();
    }

    HINTERNET opened = InternetOpenUrl(inet,
                                       LPCWSTR(url.utf16()),
                                       nullptr,
                                       0,
                                       INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_COOKIES
                                            | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI
                                            | INTERNET_FLAG_RELOAD,
                                       0);
    if (!opened)
    {
        qWarning() << "InternetOpenUrl() failed:" << cdda::os_error_to_str(GetLastError());
        InternetCloseHandle(inet);
        return QByteArray();
    }

    QByteArray retval;
    char buf[1024];
    DWORD bytesRead;
    BOOL success;
    do
    {
        success = InternetReadFile(opened,
                                   buf,
                                   DWORD(sizeof(buf)),
                                   &bytesRead);
        if (success && bytesRead)
            retval.append(buf, int(bytesRead));
    } while (success && bytesRead > 0);

    if (!success)
    {
        qWarning() << "InternetReadFile() failed:" << cdda::os_error_to_str(GetLastError());
    }

    return retval;
}

namespace MusicBrainz {

UrlDownloader::UrlDownloader(const QString &url, QObject *parent) : QObject(parent)
{
    auto future = QtConcurrent::run(downloadWinInet, url);
    QFutureWatcher<QByteArray> *w  = new QFutureWatcher<QByteArray>(this);
    connect(w, &QFutureWatcherBase::finished, this, [=]() {
        this->m_data = w->result();
        emit this->downloaded(m_data);
    });
    connect(w, &QFutureWatcherBase::finished, w, &QObject::deleteLater);
    w->setFuture(future);
}
#else

#include <QNetworkAccessManager>
#include <QNetworkReply>


namespace MusicBrainz {

UrlDownloader::UrlDownloader(const QString &url, QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::finished, this, &UrlDownloader::fileDownloaded);

    QNetworkRequest req;
    req.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setUrl(QUrl(url));
    req.setRawHeader("User-Agent", "KuemCddaExtract/0.0 (jonas@kuemmerlin.eu)");

    m_nam->get(req);
}

void UrlDownloader::fileDownloaded(QNetworkReply *reply)
{
    m_data = reply->readAll();
    reply->deleteLater();

    emit downloaded(m_data);
}
#endif

} // namespace MusicBrainz
