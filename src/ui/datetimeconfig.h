#ifndef PAGEDATETIME_H
#define PAGEDATETIME_H

#include <QWidget>
#include <QLabel>
#include <QTimeLine>
#include <QDebug>

namespace Ui {
class PageDatetime;
}

enum ITEMTYPE {
    ITEM_YEAR,
    ITEM_MONTH,
    ITEM_DAY,
    ITEM_HOUR,
    ITEM_MINUTE,
    ITEM_AMPM,  //AMPM
};

/*!
 * \brief  The ScrollWidget class 滚动条
 * 实现滚动的操作
 * \author 5005965
 * \date   2018.7.26
 */

class ScrollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollWidget(ITEMTYPE type, int value, int min, int max, QWidget *parent = 0);
    ~ScrollWidget();

    //获取当前的设定值
    int getValue() { return m_curValue; }  //获取值

    void setWidth(int w)
    {
        int h = height();
        resize(QSize(w, h));
        wlabel = w;
    }

    //如果是日期设置滚动条，则需要根据当前的月份年份来动态的更新范围
    void setDayRange(int max)
    {
        if(ITEM_DAY != m_type)
            return;

        m_valuemax = max;
        m_curValue = m_valuemin;
        updateScene();
    }

protected:
    virtual void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *event);

public slots:
    void switchDown(); //向下切换
    void switchUp(); //向上切换

private slots:
    void slotShiftPage(int frame); //界面切换

private:
    void updateValue(); //更新字面值
    void initScene(); //初始化界面
    void updateScene(); //更新界面

signals:
    void valueChange(); //当前值发生切换的消息

private:
    QList<QPoint>   m_positions; //原值的固定位置
    QList<QLabel*>  m_labels; //新建QLabel显示字面值

    bool    m_mousePressed;
    QPoint 	m_startPoint;
    qreal   m_distance;
    qreal   m_pageOffset;

    ITEMTYPE    m_type;

    QList<QString>   m_strings;  //字面值列表
    int   m_curValue; //当前值
    int   m_curIndex; //

    int  wlabel; //每个显示控件的宽度
    int  hlabel; //每个显示控件的高度

    int  m_valuemin; //最小值
    int  m_valuemax; //最大值

};

/*!
 * \brief  The PageDatetime class 日期时间设置界面
 * \author 5005965
 * \date   2018.7.26
 */
class PageDatetime : public QWidget
{
    Q_OBJECT

public:
    explicit PageDatetime(QWidget *parent = 0);
    ~PageDatetime();

private slots:
    void on_button_cancel_clicked();
    void on_button_time_clicked(); //切换到日期子窗口
    void on_button_date_clicked(); //切换到时间子窗口
    void on_button_confirm_clicked();

public slots:
    void updateRange(); //更新范围

protected:

signals:
    void goback();

private:
    void updateButtonState();

private:
    Ui::PageDatetime    *ui;

    ScrollWidget        *m_yearWidget; //年滚动条
    ScrollWidget        *m_monthWidget; //月滚动条
    ScrollWidget        *m_dayWidget; //日滚动条

    ScrollWidget        *m_hourWidget; //小时滚动条
    ScrollWidget        *m_minuteWidget; //分滚动条
    ScrollWidget        *m_formatWidget; //12小时制AM/PM

    int  type; //0: 设置日期 1: 设置时间

};

#endif // PAGEDATETIME_H
