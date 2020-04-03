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

#ifndef ICONWIDGET_H
#define ICONWIDGET_H

#include <QWidget>
#include <QIcon>

class IconWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QIcon::State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QIcon::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

public:
    explicit IconWidget(QWidget *parent = nullptr);

    QIcon icon() const { return m_icon; }
    QIcon::State state() const { return m_state; }
    QIcon::Mode mode() const { return m_mode; }
    Qt::Alignment alignment() const { return m_alignment; }
signals:
    void iconChanged(const QIcon &icon);
    void stateChanged(QIcon::State state);
    void modeChanged(QIcon::Mode mode);
    void alignmentChanged(Qt::Alignment alignment);

public slots:
    void setIcon(const QIcon &icon);
    void setState(QIcon::State state);
    void setMode(QIcon::Mode mode);
    void setAlignment(Qt::Alignment alignment);

private:
    QIcon m_icon;
    QIcon::State m_state { QIcon::Off };
    Qt::Alignment m_alignment { Qt::AlignCenter };
    QIcon::Mode m_mode { QIcon::Normal };

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // ICONWIDGET_H
