#include "iconwidget.h"
#include <QPainter>
#include <QPaintEvent>

IconWidget::IconWidget(QWidget *parent) : QWidget(parent)
{

}

void IconWidget::setIcon(const QIcon &icon)
{
    m_icon = icon;
    emit iconChanged(icon);

    this->update();
}

void IconWidget::setState(QIcon::State state)
{
    if (m_state != state)
    {
        m_state = state;
        emit stateChanged(state);

        this->update();
    }
}

void IconWidget::setMode(QIcon::Mode mode)
{
    if (m_mode != mode)
    {
        m_mode = mode;
        emit modeChanged(mode);

        this->update();
    }
}

void IconWidget::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment != alignment)
    {
        m_alignment = alignment;
        emit alignmentChanged(alignment);

        this->update();
    }
}


void IconWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.eraseRect(event->rect());
    painter.setRenderHint(QPainter::Antialiasing);

    // calculate aligned rect
    int size = std::min(width(), height());
    int x, y;

    if (m_alignment.testFlag(Qt::AlignLeft))
        x = 0;
    else if (m_alignment.testFlag(Qt::AlignRight))
        x = width() - size;
    else
        x = (width() - size)/2;

    if (m_alignment.testFlag(Qt::AlignTop))
        y = 0;
    else if (m_alignment.testFlag(Qt::AlignBottom))
        y = height() - size;
    else
        y = (height() - size)/2;

    m_icon.paint(&painter, x, y, size, size, m_alignment, m_mode, m_state);
}
