#include "vsswipeview.h"
#include "vsglobal.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

VSSwipeView::VSSwipeView(QWidget* parent)
    :QGraphicsView(parent)
    ,m_mousePressed(false)
    ,m_pageOffset(0)
    ,m_bNoZoom(true)
    , m_pPrevImage(NULL)
    , m_pCurrentImage(NULL)
    , m_pNextImage(NULL)
{
    setGeometry(QRect(0,0,w_desk,h_desk)); //全屏显示

    setScene(&m_scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);

    connect(&m_pageAnimator,SIGNAL(frameChanged(int)), SLOT(slotShiftPage(int)));
    connect(&m_pageAnimator,SIGNAL(finished()), SLOT(slotUpdateFile()));
    m_pageAnimator.setDuration(500);
    m_pageAnimator.setCurveShape(QTimeLine::EaseInCurve);
    m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);

    m_pCurrentImage = new ImageLabel();
    m_pPrevImage = new ImageLabel();
    m_pNextImage = new ImageLabel();

    clearWidget();
    addWidget(m_pPrevImage);
    addWidget(m_pCurrentImage);
    addWidget(m_pNextImage);

    for(int i=0; i<3; i++)
    {
        m_positions << QPointF(i*this->width(),0);
    }

    layoutScene();

}

void VSSwipeView::mousePressEvent(QMouseEvent *event)
{
    emit imageClicked();
    //缩放还原状态，支持切换，否则拖动查看放大的图片
    if(m_bNoZoom)
    {
        m_startPoint = event->pos();
        m_distance = 0;
        m_mousePressed = true;
    }
    else
    {
        QGraphicsView::mousePressEvent(event);
    }
}

void VSSwipeView::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_mousePressed)
        return;

    emit imageDraged();
    //缩放还原状态，支持切换，否则拖动查看放大的图片
    if(m_bNoZoom)
    {
        int dx = event->pos().x() - m_startPoint.x();
        if(!m_prevEnable && dx > 0) //can not swipe prev
            return;
        else if(!m_nextEnable && dx < 0) //can not swipe next
            return;
        m_distance += dx;
        m_startPoint = event->pos();
        int frame = m_distance*1000/width();
        slotShiftPage(frame);
    }
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void VSSwipeView::mouseReleaseEvent(QMouseEvent *event)
{
    m_mousePressed = false;

    //判断滑动的距离，达到三分之一的距离进行切换
    if(m_bNoZoom)
    {
        m_mousePressed = false;
        int start=0,end=0;
        if(m_distance > 0)
        {
            if(m_distance >= this->width()/4 && true == m_prevEnable)
            {
                start = m_distance * 1000/width();
                m_pageOffset = 1;
                end  = 1000;
            }
            else
            {
                start = m_distance * 1000/width();
                end = 0;
            }
        }
        else if(m_distance < 0)
        {
            if(m_distance <= -this->width()/4 && true == m_nextEnable)
            {
                start = m_distance * 1000/width();
                m_pageOffset = -1;
                end = -1000;
            }
            else
            {
                start = m_distance * 1000/this->width();
                end = 0;
            }
        }
        if(QTimeLine::NotRunning == m_pageAnimator.state())
        {
            m_pageAnimator.setFrameRange(start,end);
            m_pageAnimator.start();
        }
    }
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void VSSwipeView::layoutScene()
{
    m_scene.setSceneRect(0,0, this->width()*3, this->height());
#if 0
    //将图片查看器布局到滑动界面中
    if(true == m_prevEnable && true == m_nextEnable)
    {
        centerOn(this->width()*3/2, this->height()/2);
    }
    else if(false == m_prevEnable)
    {
        centerOn(this->width()/2, this->height()/2);
    }
    else if(false == m_nextEnable)
    {
        centerOn(this->width()*5/2, this->height()/2);
    }
#endif

    for(int i=0; i<3; i++)
    {
        m_items[i]->widget()->setGeometry(i*this->width(), 0, this->width(), this->height());
    }

}

void VSSwipeView::setFile(QFileInfo prevFile, QFileInfo currentFile, QFileInfo nextFile)
{
    m_pPrevImage->setImage(prevFile);
    m_pCurrentImage->setImage(currentFile);
    m_pNextImage->setImage(nextFile);

    updateCenter();
}

void VSSwipeView::updateCenter()
{
    //将图片查看器布局到滑动界面中
    if(true == m_prevEnable && true == m_nextEnable)
    {
        centerOn(this->width()*3/2, this->height()/2);
    }
    else if(false == m_prevEnable)
    {
        centerOn(this->width()/2, this->height()/2);
    }
    else if(false == m_nextEnable)
    {
        centerOn(this->width()*5/2, this->height()/2);
    }
    else
    {
        centerOn(this->width()*2, this->height()/3);
    }

    for(int i=0; i<3; i++)
    {
        m_items[i]->widget()->setGeometry(i*this->width(), 0, this->width(), this->height());
    }
}

void VSSwipeView::slotShiftPage(int frame)
{
    qreal offset= (width() * frame) / 1000;

    for(int i = 0; i < 3; i++)
    {
        QPointF point = m_positions[i];
        m_items[i]->setPos(point + QPointF(offset, 0));
    }
}

//切换完成后，发送切换消息
void VSSwipeView::slotUpdateFile()
{
    if(-1 == m_pageOffset)
    {
        emit sigSwitchNext();
    }
    else if(1 == m_pageOffset)
    {
        emit sigSwitchPrev();
    }
    m_pageOffset = 0;
}

//不支持向前切换
void VSSwipeView::slotFirstImage()
{
    m_prevEnable = false;
    m_nextEnable = true;
}

//不支持向后切换
void VSSwipeView::slotLastImage()
{
    m_nextEnable = false;
    m_prevEnable = true;
}

void VSSwipeView::slotOneImage()
{
    m_nextEnable = false;
    m_prevEnable = false;
}

//支持前后切换
void VSSwipeView::slotResetSwitchEnable()
{
    m_nextEnable = true;
    m_prevEnable = true;
}

//当前图片放大，不支持切换
void VSSwipeView::slotCurrentImageZoomIn()
{
    m_bNoZoom = false;
}

//当前图片缩放还原，支持切换
void VSSwipeView::slotCurrentImageNoneZoom()
{
    m_bNoZoom = true;
}
