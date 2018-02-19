#ifndef CLICKABLEBUDDYLABEL_H
#define CLICKABLEBUDDYLABEL_H

#include <QLabel>

class ClickableBuddyLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableBuddyLabel(QWidget *parent = nullptr);
    ~ClickableBuddyLabel() override;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // CLICKABLEBUDDYLABEL_H
