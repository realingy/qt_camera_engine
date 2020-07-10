#include "hideframe.h"
#include <QMouseEvent>

#include "vsglobal.h"
#include <QMouseEvent>
#include <QDebug>

HideSlider::HideSlider(QWidget *parent)
    : QSlider(parent)
    , m_mousePress(false)
    , m_bShow(false)
{
}

void HideSlider::mousePressEvent(QMouseEvent *event)
{
    m_mousePress = true;
    m_startPoint = event->pos();

    m_distance = 0;
    m_value = value();
}

void HideSlider::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePress)
    {
        if(!m_bShow)
        {
            emit showSlider();
            m_bShow = true;
        }

        m_distance += m_startPoint.y() - event->pos().y();
        if(m_distance >= height()/10 || m_distance <= -height()/10)
        {
            m_value += m_distance*10/height();
            m_distance = 0;
        }

        if(m_value < 1)
            m_value = 1;
        else if(m_value > 10)
            m_value = 10;

        setValue(m_value);
        m_startPoint = event->pos();
    }
}

void HideSlider::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_mousePress = false;
    emit hideSlider();
    m_bShow = false;
}

//***********************************************************
//***********************************************************
//***********************************************************
HideFrame::HideFrame(QWidget *parent)
    : QFrame(parent)
    , m_sliderShow(false)
{

}

void HideFrame::mousePressEvent(QMouseEvent*event)
{
    m_mousePress = true;
    m_startPoint = event->pos();
    m_distance = 0;
//    QFrame::mousePressEvent(event);
}

void HideFrame::mouseMoveEvent(QMouseEvent*event)
{
    if(m_mousePress)
    {
        if(!m_sliderShow)
        {
            emit showSlider();
            m_sliderShow = true;
        }

        m_distance += m_startPoint.y() - event->pos().y();
        if(m_distance >= height()/10 || m_distance <= -height()/10)
        {
            emit sliderValueChange(m_distance*10/height());
            m_distance = 0;
        }

        m_startPoint = event->pos();
    }
//    QFrame::mouseMoveEvent(event);
}

void HideFrame::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_mousePress = false;
    m_sliderShow = false;
    emit hideSlider();
//    QFrame::mouseReleaseEvent(event);
}
