#ifndef VSSWIPEVIEW_H
#define VSSWIPEVIEW_H

#include <QWidget>
#include <QTimeLine>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include "imagelabel.h"

class VSSwipeView : public QGraphicsView
{
    Q_OBJECT
public:
    VSSwipeView(QWidget* parent = 0);

public:
    void layoutScene();
    //清空Scene和图片查看器
    void clearWidget()
    {
        m_scene.clear();
        m_items.clear();
    }

    //添加一个图片查看器控件到Scene中，并将Scene添加到滑动界面
    void addWidget(QWidget* widget)
    {
        m_items.append(m_scene.addWidget(widget));
    }

    void setFile(QFileInfo prevFile, QFileInfo currentFile, QFileInfo nextFile);
    void zoomOut(int scale) { m_pCurrentImage->zoomOut(scale); }
    void zoomIn(int scale) { m_pCurrentImage->zoomIn(scale);}
    void zoomReset() { m_pCurrentImage->reset(); }
    void updateCenter();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void sigSwitchPrev();
    void sigSwitchNext();
    void imageClicked();
    void imageDraged();

public slots:
    void slotResetSwitchEnable();
    void slotFirstImage();
    void slotLastImage();
    void slotOneImage();
    void slotCurrentImageZoomIn();
    void slotCurrentImageNoneZoom();

private slots:
    void slotShiftPage(int frame);
    void slotUpdateFile();

private:
    QGraphicsScene 	m_scene;
    QTimeLine 		m_pageAnimator;
    QList<QGraphicsProxyWidget *> m_items;
    QList<QPointF> 	m_positions;

    ImageLabel      *m_pCurrentImage;
    ImageLabel      *m_pPrevImage;
    ImageLabel      *m_pNextImage;

    QPoint 			m_startPoint;
    bool			m_mousePressed;
    qreal 			m_distance;
    qreal 			m_pageOffset;

    bool			m_prevEnable;
    bool			m_nextEnable;
    bool			m_bNoZoom;

};

#endif // VSSWIPEVIEW_H
