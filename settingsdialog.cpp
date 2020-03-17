#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QSettings settings;

    bool p = settings.value(QStringLiteral("Paranoia Mode"), QStringLiteral("paranoia")).toString() == QStringLiteral("paranoia");
    ui->cbEnableParanoia->setChecked(p);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::accept()
{
    QSettings settings;

    if (ui->cbEnableParanoia->isChecked()) {
        settings.setValue(QStringLiteral("Paranoia Mode"), QStringLiteral("paranoia"));
    } else {
        settings.setValue(QStringLiteral("Paranoia Mode"), QStringLiteral("fast"));
    }


    QDialog::accept();
}
