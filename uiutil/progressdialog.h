#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

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

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;

    // QDialog interface
public slots:
    void reject() override;
};

#endif // PROGRESSDIALOG_H
