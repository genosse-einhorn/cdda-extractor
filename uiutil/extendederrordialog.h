#ifndef EXTENDEDERRORDIALOG_H
#define EXTENDEDERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ExtendedErrorDialog;
}

class ExtendedErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtendedErrorDialog(QWidget *parent = 0);
    ~ExtendedErrorDialog();

    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString details READ details WRITE setDetails NOTIFY detailsChanged)

    QString message() const;
    QString details() const;

    static void show(QWidget *parent, const QString &message, const QString &details);
signals:
    void messageChanged(const QString &newMessage);
    void detailsChanged(const QString &newDetails);

public slots:
    void setMessage(const QString &message);
    void setDetails(const QString &details);

private:
    Ui::ExtendedErrorDialog *ui;
};

#endif // EXTENDEDERRORDIALOG_H
