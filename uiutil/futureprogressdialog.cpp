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
