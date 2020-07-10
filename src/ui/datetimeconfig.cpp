#include "datetimeconfig.h"
#include "ui_datetimeconfig.h"
#include "configmanager.h"
#include <QBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QDesktopWidget>
#include <QDateTime>
#include <QPainter>

/*!
 * \brief PageDatetime::PageDatetime
 * \param parent
 */
PageDatetime::PageDatetime(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PageDatetime)
    , m_yearWidget(NULL)
    , m_monthWidget(NULL)
    , m_dayWidget(NULL)
    , m_hourWidget(NULL)
    , m_minuteWidget(NULL)
    , m_formatWidget(NULL)
    , type(0) //m_isDate
{
    ui->setupUi(this);
    QRect desktop = QApplication::desktop()->geometry();
    desktop.moveTo(QPoint(0, 0));
    setGeometry(desktop);

    ui->stackedWidget->setCurrentWidget(ui->pagedate);
    ui->button_date->setCheckable(true);
    ui->button_time->setCheckable(true);
    updateButtonState();

    QString year = QDateTime::currentDateTime().toString("yyyy");
    QString month = QDateTime::currentDateTime().toString("MM");
    QString day = QDateTime::currentDateTime().toString("dd");

    if(month.startsWith(QString("0")))
    {
        month.remove(0,1);
    }
    if(day.startsWith(QString("0")))
    {
        day.remove(0,1);
    }

    QString hour = QDateTime::currentDateTime().toString("hh");
    QString minute = QDateTime::currentDateTime().toString("mm");

    if(hour.startsWith(QString("0")))
    {
        hour.remove(0,1);
    }
    if(minute.startsWith(QString("0")))
    {
        minute.remove(0,1);
    }

    m_yearWidget = new ScrollWidget(ITEM_YEAR, year.toInt(), 2010, 2099, ui->widget_g1);
    connect(m_yearWidget, SIGNAL(valueChange()), SLOT(updateRange())); //updateDayRange

    m_monthWidget = new ScrollWidget(ITEM_MONTH, month.toInt(), 1, 12, ui->widget_g2);
    connect(m_monthWidget, SIGNAL(valueChange()), SLOT(updateRange()));

    m_dayWidget = new ScrollWidget(ITEM_DAY, day.toInt(), 1, 30, ui->widget_g3);
    updateRange(); //手动调用一次更新范围的槽函数更新日期范围

    m_minuteWidget = new ScrollWidget(ITEM_MINUTE, minute.toInt(), 0, 59, ui->time_g2);

    //根据当前的12/24小时制选择小时滚动条的范围和PM/AM的显示
    QVariant b12hformat;
    if(ConfigInstance->readSet(ITEM_TIME_FORMAT,b12hformat))
    {
        if(0 == b12hformat.toInt())
        {
            ui->time_g3->hide(); //
            m_hourWidget = new ScrollWidget(ITEM_HOUR, hour.toInt(), 0, 23, ui->time_g1);
            m_hourWidget->setWidth(172);
            m_minuteWidget->setWidth(172);
        }
        else if(1 == b12hformat.toInt())
        {
            m_hourWidget = new ScrollWidget(ITEM_HOUR, hour.toInt(), 1, 12, ui->time_g1);
            m_formatWidget = new ScrollWidget(ITEM_AMPM, 0, 0, 1, ui->time_g3);
        }
    }
}

PageDatetime::~PageDatetime()
{
    delete ui;
}

void PageDatetime::on_button_cancel_clicked()
{
    emit goback();
}

void PageDatetime::updateButtonState()
{
    if(!type)
    {
        ui->button_date->setChecked(true);
        ui->button_time->setChecked(false);
    }
    else
    {
        ui->button_time->setChecked(true);
        ui->button_date->setChecked(false);
    }
}

void PageDatetime::on_button_time_clicked()
{
    type = 1;
    updateButtonState();
    ui->stackedWidget->setCurrentWidget(ui->pagetime);
}

void PageDatetime::on_button_date_clicked()
{
    type = 0;
    updateButtonState();
    ui->stackedWidget->setCurrentWidget(ui->pagedate);
}

/*!
 * \brief PageDatetime::on_button_confirm_clicked
 * 点击确定，根据当前的年月日和时间当前值设置RTC
 */
void PageDatetime::on_button_confirm_clicked()
{
    int year = m_yearWidget->getValue();
    int month = m_monthWidget->getValue();
    int day = m_dayWidget->getValue();

    int hour = m_hourWidget->getValue();
    int minute = m_minuteWidget->getValue();

    QString datetime = QString("\"%1-%2-%3 %4:%5\"").arg(year).arg(month).arg(day).arg(hour).arg(minute);

    QString cmd1 = QString("date -s %1").arg(datetime);
    system(cmd1.toLatin1().data());

    QString cmd2 = QString("hwclock -w ");
    system(cmd2.toLatin1().data());

    emit goback();
}

/*!
 * \brief PageDatetime::updateRange 年份和月份发生更改时更新日期的范围
 */
void PageDatetime::updateRange()
{
    int year = m_yearWidget->getValue();
    int month = m_monthWidget->getValue();
    bool leap;

    if(0 == year%400 || ( 0 == year%4 && 0 != year%100)) //判断是否为闰年
    {
        leap = true;
    }
    else
    {
        leap = false;
    }

#if 0
    if(2 == month && true == leap)
    {
        m_dayWidget->setDayRange(29); //闰年2月
    }
    else if(2 == month && false == leap)
    {
        m_dayWidget->setDayRange(28); //平年2月
    }
#endif
    if(2 == month)
    {
        if(false == leap)
            m_dayWidget->setDayRange(28); //平年2月
        else
            m_dayWidget->setDayRange(29); //闰年2月
    }
    else if(4 == month || 6 == month || 9 == month || 11 == month)
    {
        m_dayWidget->setDayRange(30); //30天月份
    }
    else
    {
        m_dayWidget->setDayRange(31); //31天月份
    }
}

/*!
 * \brief ScrollWidget::ScrollWidget 滚动界面
 * \param type 类型
 * \param value 当前值
 * \param min 最小值
 * \param max 最大值
 */
ScrollWidget::ScrollWidget(ITEMTYPE type, int value, int min, int max, QWidget *parent)
    : QWidget(parent)
    , m_pageOffset(0)
    , m_type(type)
    , m_curValue(value)
    , m_valuemin(min)
    , m_valuemax(max)
{
#ifdef __arm__
    resize(QSize(108, 600));
#else
    resize(QSize(240, 1200));
#endif

    updateValue(); //根据当前字面值范围更新字面值列表

    wlabel = width();
    hlabel = height()/20;
    initScene();
}

ScrollWidget::~ScrollWidget()
{
}

void ScrollWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform );

    painter.drawText(this->geometry(), QString("test"));

//    QPoint center(m_pix.width()/2,m_pix.height()/2);
//    painter.translate(m_orgPoint - center);

//    painter.drawPixmap(0, 0, m_pix.width(), m_pix.height(), m_pix);
}

void ScrollWidget::mousePressEvent(QMouseEvent *event)
{
    m_mousePressed = true;
    m_startPoint = event->pos();
    m_distance = 0;
    QWidget::mousePressEvent(event);
}

void ScrollWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePressed)
    {
        int dy = event->pos().y() - m_startPoint.y();
        m_distance += dy;
        m_startPoint = event->pos();
        int frame = m_pageOffset*1000 + m_distance*1000/hlabel;
        slotShiftPage(frame);
    }
    QWidget::mouseMoveEvent(event);
}

void ScrollWidget::resizeEvent(QResizeEvent*event)
{
    for(int i = 0; i < 20; i++)
    {
        m_labels[i]->resize(wlabel, hlabel);
    }

    QWidget::resizeEvent(event);
}

/*!
 * \brief ScrollWidget::mouseReleaseEvent
 * 根据当前的滑动距离决定切换几格
 */
void ScrollWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_mousePressed = false;
    if(m_distance > 0)
    {
        if(m_distance >= hlabel/4)
        {
            int count = m_distance/hlabel;
            if(!count)
            {
                switchUp(); //切换一格
            }
            else
            {
                while(count)
                {
                    switchUp(); //切换多格
                    count--;
                }
            }
        }
        else
        {
            for(int i = 0; i < 20; i++)
            {
                //滚动的距离不够恢复原来的位置
                m_labels[i]->move(m_positions[i]);
            }
        }
    }
    else if(m_distance < 0)
    {
        if(m_distance <= -hlabel/4)
        {
            int count = -m_distance/hlabel;
            if(!count)
            {
                switchDown(); //切换一格
            }
            else
            {
                while(count)
                {
                    switchDown(); //切换多格
                    count--;
                }
            }
        }
        else
        {
            for(int i = 0; i < 20; i++)
            {
                //滚动的距离不够恢复原来的位置
                m_labels[i]->move(m_positions[i]);
            }
        }
    }

    emit valueChange();

    QWidget::mouseReleaseEvent(event);
}

//切换一格，更新当前值，并更新界面显示
void ScrollWidget::switchDown()
{
    if(m_curValue < m_valuemax)
        m_curValue = m_curValue + 1;
    else
        m_curValue = m_valuemin;

    updateScene();
}

//切换一格，更新当前值，并更新界面显示
void ScrollWidget::switchUp()
{
    if(m_curValue > m_valuemin)
        m_curValue = m_curValue - 1;
    else if(m_curValue == m_valuemin)
        m_curValue = m_valuemax;

    updateScene();
}

/*!
 * \brief ScrollWidget::slotShiftPage
 * 实现滚动过程中的滚动效果
 */
void ScrollWidget::slotShiftPage(int frame)
{
    qreal offset = (hlabel*frame)/1000;
    for(int i = 0; i < 20; i++)
    {
        QPoint point = m_positions[i];
        point += QPoint(0, offset);
        m_labels[i]->move(point);
    }
}

/*!
 * \brief ScrollWidget::updateValue
 * 更新字面值
 */
void ScrollWidget::updateValue()
{
    m_strings.clear(); //清空数字列表

    if(ITEM_AMPM == m_type)
    {
        for(int i = 0; i < 4; i++)
        {
            m_strings.append(QString(" "));
        }
        m_strings.append(QString("0"));
        m_strings.append(QString("1"));
        for(int i = 0; i < 4; i++)
        {
            m_strings.append(QString(" "));
        }
        return;
    }

    //循环显示，保证字面值列表的元素数量为20个，当前值前面10个后面9个
    if(m_curValue - m_valuemin < 10)
    {
        int d = 10 - (m_curValue - m_valuemin);
        for(int i = 1; i <= d; i++)
        {
            m_strings.append(QString::number(i + m_valuemax - d));
        }
        for(int value = m_valuemin; value < m_curValue+10; value++)
        {
            m_strings.append(QString::number(value));
        }
    }
    else if(m_valuemax - m_curValue < 9)
    {
        int d = 9 - (m_valuemax - m_curValue);
        for(int value = m_curValue-10; value <= m_valuemax; value++)
        {
            m_strings.append(QString::number(value));
        }
        for(int i = 0; i < d; i++)
        {
            m_strings.append(QString::number(m_valuemin + i));
        }
    }
    else
    {
        for(int value = m_curValue-10; value < m_curValue+10; value++)
        {
            m_strings.append(QString::number(value));
        }
    }

}

/*!
 * \brief ScrollWidget::updateScene
 * 切换成功后，调用此方法更新显示，各个QLabel恢复到原始位置
 */
void ScrollWidget::updateScene()
{
    updateValue(); //更新字面值队列

    //更新各个label上显示的数字
    for(int i = 0; i < 20; i++)
    {
        QString string = m_strings.value(i);
        m_labels[i]->setText(string);
        if(string.toInt() == m_curValue)
        {
            m_labels[i]->setStyleSheet(QString("color: white; font: 20px;")); //当前值显示为白色
            m_curIndex = i;
        }
        else
        {
            m_labels[i]->setStyleSheet(QString("color: grey; font: 20px;"));
        }
        m_labels[i]->move(m_positions[i]);
    }
}

/*!
 * \brief ScrollWidget::initScene
 * 初始化界面，在构造函数中调用一次，并记录各个QLabel的位置
 */
void ScrollWidget::initScene()
{
    updateValue(); //更新字面值队列
    m_positions.clear(); //位置清空
    for(int i = 0; i < 20; i++)
    {
        QString string = m_strings.value(i);
        QLabel *label = new QLabel(this);
        label->setText(string);
        if(ITEM_AMPM == m_type)
        {
            if(string == QString("0"))
                label->setText(QString("PM"));
            else if(string == QString("1"))
                label->setText(QString("AM"));
            else
                label->setText(string);
        }
        else
        {
            label->setText(string);
        }
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        if(string.toInt() == m_curValue)
        {
            label->setStyleSheet(QString("color: white; font: 20px;"));
            m_curIndex = i;
        }
        else
        {
            label->setStyleSheet(QString("color: grey; font: 20px;"));
        }
        label->setGeometry(QRect(0, hlabel*i, wlabel, hlabel));
        label->show();
        m_labels.append(label);
        m_positions.append(QPoint(0, hlabel*i));
    }
    int offset = (m_curIndex - 3)*hlabel;
    for(int i = 0; i < 20; i++)
    {
        m_positions[i] -= QPoint(0, offset);
        m_labels[i]->move(m_positions[i]);
    }
}
