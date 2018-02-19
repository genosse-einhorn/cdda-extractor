#ifndef PARANOIA_TOC_READER2_H
#define PARANOIA_TOC_READER2_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include "toc.h"

namespace cdda {

class paranoia_toc_reader2 : public QObject
{
    Q_OBJECT
public:
    explicit paranoia_toc_reader2(QObject *parent = nullptr);

signals:
    void success(const std::vector<toc_track> &tracks);
    void error(const QString &message);

public slots:
    void start();

private:

};

} // namespace cdda

#endif // PARANOIA_TOC_READER2_H
