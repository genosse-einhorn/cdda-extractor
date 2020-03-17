#ifndef MUSICBRAINZASKDIALOG_H
#define MUSICBRAINZASKDIALOG_H

#include <QDialog>

namespace Ui {
class MusicBrainzAskDialog;
}

class MusicBrainzAskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MusicBrainzAskDialog(QWidget *parent = nullptr);
    ~MusicBrainzAskDialog();

    static bool showAskDialog(QWidget *owner);
    static bool downloadOkMaybeAsk(QWidget *parentWindow);
    static bool downloadOk(void);

private slots:
    void on_yesBtn_clicked();

    void on_noBtn_clicked();

private:
    Ui::MusicBrainzAskDialog *ui;
};

#endif // MUSICBRAINZASKDIALOG_H
