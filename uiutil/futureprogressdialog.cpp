#include "futureprogressdialog.h"

FutureProgressDialog::FutureProgressDialog(QWidget *parent) : ProgressDialog(parent)
{
    connect(&m_watcher, &QFutureWatcherBase::finished, this, &QWidget::hide);
    connect(&m_watcher, &QFutureWatcherBase::started, this, &QWidget::show);
    connect(&m_watcher, &QFutureWatcherBase::progressRangeChanged, this, &ProgressDialog::setRange);
    connect(&m_watcher, &QFutureWatcherBase::progressValueChanged, this, &ProgressDialog::setValue);
    connect(&m_watcher, &QFutureWatcherBase::progressTextChanged, this, &ProgressDialog::setLabelText);
    connect(this, &ProgressDialog::canceled, &m_watcher, &QFutureWatcherBase::cancel);
}

void FutureProgressDialog::setFuture(const QFuture<void> &future)
{
    this->uncancel();
    setRange(0, 0);
    setValue(0);
    setLabelText(QString());
    m_watcher.setFuture(future);
}
