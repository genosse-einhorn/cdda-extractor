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
