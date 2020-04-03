// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
