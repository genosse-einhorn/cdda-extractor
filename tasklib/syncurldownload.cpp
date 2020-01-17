#include "syncurldownload.h"

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>

namespace TaskRunner {

QByteArray downloadUrl(const QString &url, const CancelToken &cancelToken)
{
    QEventLoop loop;
    QNetworkAccessManager nam;
    CancelWatcher cw;
    QObject::connect(&nam, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    QNetworkRequest req;
    req.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setUrl(QUrl(url));
    req.setRawHeader("User-Agent", "KuemCddaExtract/0.0 (jonas@kuemmerlin.eu)");

    QNetworkReply *reply = nam.get(req);
    QObject::connect(&cw, &CancelWatcher::canceled, reply, &QNetworkReply::abort);

    cw.setToken(cancelToken);
    loop.exec();

    QByteArray data = reply->readAll();
    delete reply;

    return data;
}

} // namespace TaskRunner
