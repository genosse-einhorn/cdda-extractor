#ifndef TOC_FINDER_H
#define TOC_FINDER_H

#include <QObject>

#include "toc.h"

namespace cdda {

class toc_finder : public QObject
{
    Q_OBJECT
public:
    explicit toc_finder(QObject *parent = nullptr);

signals:
    void success(const QString &device, const cdda::toc &toc);
    void error(const QString &message);

public slots:
    void start();

private:
    struct toc_result
    {
        QStringList log;
        QString device;
        cdda::toc toc;
    };

    static toc_result readToc();
};

} // namespace cdda

#endif // TOC_FINDER_H
