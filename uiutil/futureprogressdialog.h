#ifndef FUTUREPROGRESSDIALOG_H
#define FUTUREPROGRESSDIALOG_H

#include "progressdialog.h"
#include <QFutureWatcher>

class FutureProgressDialog : public ProgressDialog
{
    Q_OBJECT
public:
    FutureProgressDialog(QWidget *parent = nullptr);

public slots:
    void setFuture(const QFuture<void> &future);

private:
    QFutureWatcher<void> m_watcher;
};

#endif // FUTUREPROGRESSDIALOG_H
