#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    ui->progressBarContainer->setBackgroundRole(QPalette::Base);
    ui->progressBarContainer->setForegroundRole(QPalette::Text);
    ui->progressBarContainer->setAutoFillBackground(true);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

bool ProgressDialog::isCancelAllowed() const
{
    return m_cancelAllowed;
}

int ProgressDialog::minimum() const
{
    return ui->progressBar->minimum();
}

int ProgressDialog::maximum() const
{
    return ui->progressBar->maximum();
}

int ProgressDialog::value() const
{
    return ui->progressBar->value();
}

QString ProgressDialog::labelText() const
{
    return ui->label->text();
}

bool ProgressDialog::isCanceled()
{
    return m_canceled;
}

void ProgressDialog::setMinimum(int min)
{
    ui->progressBar->setMinimum(min);
}

void ProgressDialog::setMaximum(int max)
{
    ui->progressBar->setMaximum(max);
}

void ProgressDialog::setValue(int progress)
{
    ui->progressBar->setValue(progress);
}

void ProgressDialog::setLabelText(const QString &text)
{
    ui->label->setText(text);
}

void ProgressDialog::setCancelAllowed(bool allowed)
{
    m_cancelAllowed = allowed;
    ui->buttonBox->setEnabled(allowed);
}

void ProgressDialog::cancel()
{
    m_canceled = true;
    ui->buttonBox->setEnabled(false);
    emit canceled();
}

void ProgressDialog::uncancel()
{
    m_canceled = false;
    ui->buttonBox->setEnabled(m_cancelAllowed);
}


void ProgressDialog::showEvent(QShowEvent *event)
{
    this->adjustSize();
    QDialog::showEvent(event);
}

void ProgressDialog::reject()
{
    if (m_cancelAllowed)
        this->cancel();
}
