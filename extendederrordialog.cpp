#include "extendederrordialog.h"
#include "ui_extendederrordialog.h"

#include <QIcon>

ExtendedErrorDialog::ExtendedErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtendedErrorDialog)
{
    ui->setupUi(this);

    QIcon errorIcon = QIcon::fromTheme(QStringLiteral("dialog-error"));
    if (!errorIcon.isNull())
    {
        QWindow *window = nullptr;
        if (const QWidget *nativeParent = ui->lIcon->nativeParentWidget())
            window = nativeParent->windowHandle();

        QPixmap p = errorIcon.pixmap(window, ui->lIcon->size());
        ui->lIcon->setPixmap(p);
    }
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
