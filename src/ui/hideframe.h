#ifndef HIDEFRAME_H
#define HIDEFRAME_H

#include <QFrame>
#include <QSlider>

class HideSlider : public QSlider
{
    Q_OBJECT
public:
    HideSlider(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void showSlider();
    void hideSlider();

private:
    QPoint m_startPoint;
    int    m_value;
    int    m_distance;

    bool   m_mousePress;
    bool   m_bShow;

};

//***************************************************

class HideFrame : public QFrame
{
    Q_OBJECT
public:
    HideFrame(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void showSlider();
    void hideSlider();
    void sliderValueChange(int change);

private:
    QPoint m_startPoint;
    int    m_distance;

    bool   m_mousePress;
    bool   m_sliderShow;

};

#endif // HIDEFRAME_H
