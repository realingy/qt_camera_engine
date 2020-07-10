#include "imagelabel.h"
#include <QFileInfo>
#include <QDebug>
#include <QRect>
#include "vsglobal.h"

ImageLabel::ImageLabel()
    : m_scale(MINSCALE)
{
    setGeometry(QRect(0,0,w_desk,h_desk)); //全屏显示
}

void ImageLabel::setImage(QFileInfo file)
{
    m_file = file;
    reset();
}

void ImageLabel::reset()
{
    m_scale = MINSCALE;
    scaleImage();
    m_orgPoint.setX(width()/2);
    m_orgPoint.setY(height()/2);
    update();
}

void ImageLabel::scaleImage()
{
    m_pix.load(m_file.absoluteFilePath());
    qreal scale = (qreal) m_scale / MINSCALE;
    m_pix = m_pix.scaled(scale * width(), scale * height());
}

void ImageLabel::zoomIn(int in)
{
    if(in < m_scale)
        return;
    m_scale = in;
    scaleImage();
    update();
}

void ImageLabel::zoomOut(int out)
{
    if(out > m_scale)
        return;
    m_scale = out;
    scaleImage();
    update();
}

void ImageLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_orgPoint.setX(width()/2);
    m_orgPoint.setY(height()/2);
    update();
}

void ImageLabel::mousePressEvent(QMouseEvent *event)
{
    m_startPoint = event->pos();
    update();
}

void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    m_orgPoint += event->pos() - m_startPoint;
    m_startPoint = event->pos();
    update();
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    m_orgPoint += event->pos() - m_startPoint;
    update();
}

void ImageLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing );
    painter.setRenderHint(QPainter::SmoothPixmapTransform );

    QPoint center(m_pix.width()/2,m_pix.height()/2);
    painter.translate(m_orgPoint - center);

    painter.drawPixmap(0, 0, m_pix.width(), m_pix.height(), m_pix);

}
