// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
