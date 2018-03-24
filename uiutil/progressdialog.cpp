#include "progressdialog.h"
#include "ui_progressdialog.h"
#ifdef Q_OS_WIN32
#   include <QWinTaskbarButton>
#   include <QWinTaskbarProgress>
#endif

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    ui->progressBarContainer->setBackgroundRole(QPalette::Base);
    ui->progressBarContainer->setForegroundRole(QPalette::Text);
    ui->progressBarContainer->setAutoFillBackground(true);

    this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);

#ifdef Q_OS_WIN32
    m_taskbarButton = new QWinTaskbarButton(this);
#endif
}

ProgressDialog::~ProgressDialog()
{
    delete ui;

#ifdef Q_OS_WIN32
    // if we couldn't hide the progress during hideEvent(), hide it now
    // we need to do this explicitly since we attach the taskbar button
    // to our parent window (you shouldn't ever do this but here we are)
    if (this->isVisible())
        m_taskbarButton->progress()->hide();
#endif
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

    fixupUi();
}

void ProgressDialog::setMaximum(int max)
{
    ui->progressBar->setMaximum(max);

    fixupUi();
}

void ProgressDialog::setValue(int progress)
{
    ui->progressBar->setValue(progress);

    fixupUi();
}

void ProgressDialog::setLabelText(const QString &text)
{
    ui->label->setText(text);

    fixupUi();
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

void ProgressDialog::fixupUi()
{
    ui->progressBar->setTextVisible(minimum() != maximum());

#ifdef Q_OS_WIN32
    m_taskbarButton->progress()->setRange(minimum(), maximum());
    m_taskbarButton->progress()->setValue(value());
    m_taskbarButton->progress()->show();
    m_taskbarButton->progress()->resume();
#endif
}


void ProgressDialog::showEvent(QShowEvent *event)
{
    this->adjustSize();
    QDialog::showEvent(event);

#ifdef Q_OS_WIN32
    // initialize windows taskbar progress
    if (this->isModal() && this->parentWidget())
        m_taskbarButton->setWindow(this->parentWidget()->windowHandle());
    else
        m_taskbarButton->setWindow(this->windowHandle());
    m_taskbarButton->progress()->show();
    m_taskbarButton->progress()->resume();
#endif
}

void ProgressDialog::reject()
{
    if (m_cancelAllowed)
        this->cancel();
}


void ProgressDialog::hideEvent(QHideEvent *event)
{
#ifdef Q_OS_WIN32
    m_taskbarButton->progress()->hide();
#endif

    QDialog::hideEvent(event);
}
