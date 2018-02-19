#include "clickablebuddylabel.h"

#include <QAbstractButton>

ClickableBuddyLabel::ClickableBuddyLabel(QWidget *parent): QLabel(parent)
{

}

ClickableBuddyLabel::~ClickableBuddyLabel()
{

}


void ClickableBuddyLabel::mousePressEvent(QMouseEvent *event)
{
    QLabel::mousePressEvent(event);

    emit clicked();

    if (this->buddy())
    {
        if (this->buddy()->focusPolicy() & Qt::ClickFocus)
            this->buddy()->setFocus();

        QAbstractButton *button = qobject_cast<QAbstractButton*>(this->buddy());
        if (button && button->isCheckable() && button->isEnabled())
        {
            button->setChecked(true);
        }
    }
}
