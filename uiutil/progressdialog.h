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

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class QWinTaskbarButton;

/*
 * Why not QProgressDialog? Because QProgressDialog can't be prevented from closing.
 * We want to close the dialog only after the operation has truly finished, not as soon
 * as the user clicks "Cancel". We might also not want to allow canceling at all.
 */
class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

    bool isCancelAllowed() const;
    int minimum() const;
    int maximum() const;
    int value() const;
    QString labelText() const;
    bool isCanceled();

signals:
    void canceled();

public slots:
    void setMinimum(int min);
    void setMaximum(int max);
    void setRange(int min, int max);
    void setValue(int progress);
    void setLabelText(const QString &text);
    void setCancelAllowed(bool allowed);

    // NOTE: this will not actually close the dialog, just emit the signal and set isCanceled()
    void cancel();

    // will unset isCanceled()
    void uncancel();

private:
    bool m_cancelAllowed { true };
    bool m_canceled { false };
    Ui::ProgressDialog *ui;

#ifdef Q_OS_WIN32
    QWinTaskbarButton *m_taskbarButton;
#endif

    void fixupUi();

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    // QDialog interface
public slots:
    void reject() override;
};

#endif // PROGRESSDIALOG_H
