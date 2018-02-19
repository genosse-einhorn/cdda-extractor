#ifndef COVERARTWIDGET_H
#define COVERARTWIDGET_H

#include <QWidget>

class QSvgRenderer;

class CoverArtWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CoverArtWidget(QWidget *parent = nullptr);

signals:

public slots:
    void setCover(const QImage &image);
    void resetCover();

private:
    QImage m_cover;
    QSvgRenderer *m_defaultImage;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // COVERARTWIDGET_H
