#ifndef IMAGELABEL_H
#define IMABELABEL_H

#include <QLabel>
#include <QEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QFileInfo>

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImageLabel();

public:
    void setImage(QFileInfo file);
    void zoomIn(int);
    void zoomOut(int);
    void reset();

protected:
    virtual void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

private slots:

signals:

private:
    void scaleImage();

private:
    QFileInfo    m_file;
    QPixmap		 m_pix;
    QPoint		 m_orgPoint;
    QPoint		 m_startPoint;

    quint8       m_scale;

};

#endif // IMAGELABEL_H
