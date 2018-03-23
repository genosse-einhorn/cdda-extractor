#include "extendederrordialog.h"
#include "ui_extendederrordialog.h"
#include "win32iconloader.h"

#include <QIcon>

ExtendedErrorDialog::ExtendedErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtendedErrorDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN32
    QIcon errorIcon = IconLoader::fromShellStock(SIID_ERROR);
#else
    QIcon errorIcon = this->style()->standardIcon(QStyle::SP_MessageBoxCritical);
#endif
    if (errorIcon.isNull())
        errorIcon = QIcon::fromTheme(QStringLiteral("dialog-error"), QIcon(QStringLiteral(":/dialog-error.svg")));

    ui->iwIcon->setIcon(errorIcon);
}

ExtendedErrorDialog::~ExtendedErrorDialog()
{
    delete ui;
}

QString ExtendedErrorDialog::message() const
{
    return ui->lErrorMessage->text();
}

QString ExtendedErrorDialog::details() const
{
    return ui->textBrowser->toPlainText();
}

void ExtendedErrorDialog::show(QWidget *parent, const QString &message, const QString &details)
{
    ExtendedErrorDialog dialog(parent);
    dialog.setModal(true);
    dialog.setMessage(message);
    dialog.setDetails(details);
    dialog.exec();
}

void ExtendedErrorDialog::setMessage(const QString &message)
{
    if (ui->lErrorMessage->text() != message)
    {
        ui->lErrorMessage->setText(message);
        emit messageChanged(message);
    }
}

void ExtendedErrorDialog::setDetails(const QString &details)
{
    if (this->details() != details)
    {
        ui->textBrowser->setPlainText(details);
        emit detailsChanged(details);
    }
}
