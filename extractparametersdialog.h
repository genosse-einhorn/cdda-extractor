#ifndef EXTRACTPARAMETERSDIALOG_H
#define EXTRACTPARAMETERSDIALOG_H

#include <QDialog>

namespace Ui {
class ExtractParametersDialog;
}

class ExtractParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractParametersDialog(QWidget *parent = 0);
    ~ExtractParametersDialog();

    void setAlbumInfo(const QString &title, const QString &artist);

    QString outputDir() const;
    QString format() const;
    bool paranoiaActivated() const;

private slots:
    void on_bBrowseDir_clicked();

private:
    Ui::ExtractParametersDialog *ui;
    QString m_artist;
    QString m_album;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // EXTRACTPARAMETERSDIALOG_H
