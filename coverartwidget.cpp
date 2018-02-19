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

    if (m_cover.isNull())
    {
        // default svg

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

        // then paint it into that
        m_defaultImage->render(&painter, bounds);
    }
    else
    {
        // cover image

        // calculate position for aspect-correct scaling
        QSizeF coverSize = m_cover.size();
        QSizeF widgetSize = this->size();
        qreal scaleX = widgetSize.width() / coverSize.width();
        qreal scaleY = widgetSize.height() / coverSize.height();
        qreal scale = std::min(scaleX, scaleY);
        QSizeF scaledSize = coverSize * scale;
        QRectF bounds((widgetSize.width() - scaledSize.width()) / 2,
                      (widgetSize.height() - scaledSize.height()) / 2,
                      scaledSize.width(), scaledSize.height());

        // then paint it into that
        painter.drawImage(bounds, m_cover);
    }

    painter.end();
}
