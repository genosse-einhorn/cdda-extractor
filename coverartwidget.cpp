#include "coverartwidget.h"

#include <QSvgRenderer>
#include <QPainter>

CoverArtWidget::CoverArtWidget(QWidget *parent) : QWidget(parent)
{
    m_defaultImage = new QSvgRenderer(QStringLiteral(":/cd.svg"), this);
}

void CoverArtWidget::setCover(const QImage &image)
{
    m_cover = image;
    update();
}

void CoverArtWidget::resetCover()
{
    m_cover = QImage();
    update();
}


void CoverArtWidget::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // calculate position for aspect-correct scaling
    QSizeF svgSize = m_defaultImage->defaultSize();
    QSizeF widgetSize = this->size();
    qreal scaleX = widgetSize.width() / svgSize.width();
    qreal scaleY = widgetSize.height() / svgSize.height();
    qreal scale = std::min(scaleX, scaleY);
    QSizeF scaledSize = svgSize * scale;
    QRectF bounds((widgetSize.width() - scaledSize.width()) / 2,
                  (widgetSize.height() - scaledSize.height()) / 2,
                  scaledSize.width(), scaledSize.height());

    m_defaultImage->render(&painter, bounds);

    painter.end();
}
