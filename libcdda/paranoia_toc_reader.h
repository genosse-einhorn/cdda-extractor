#ifndef PARANOIA_TOC_READER_H
#define PARANOIA_TOC_READER_H

#include "libcdda/toc.h"
#include <vector>
#include <QProcess>
#include <QStringList>

namespace cdda {

class paranoia_toc_reader : public QObject
{
    Q_OBJECT
public:
    explicit paranoia_toc_reader(QObject *parent = nullptr);

signals:
    void success(const std::vector<toc_track> &tracks);
    void error(const QString &message);

public slots:
    void start();

private:
    void readAvailable();
    void processErrord();
    void processFinished();

    enum { WAITING_FOR_START, IN_LIST, AFTER_END } m_state;
    std::vector<toc_track> m_tracks;
    QStringList m_errorLineBuf;
    QProcess m_process;
};

} // namespace cdda

#endif // PARANOIA_TOC_READER_H
