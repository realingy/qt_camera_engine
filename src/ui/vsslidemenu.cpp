#include "vsslidemenu.h"
#include "vsconfig.h"
#include "vsglobal.h"
#include "ui_vsslidemenu.h"
#include "configmanager.h"
#include <QTimer>
#include <QMouseEvent>
#include <QDateTime>

VSSlideMenu::VSSlideMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VSSlideMenu)
    , m_timer(NULL)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    ui->sliderbcklgt->setRange(BCKLGT_MIN,BCKLGT_MAX);
    ui->sliderbcklgt->setPageStep(BCKLGT_STEP);

    ui->sliderledlgt->setRange(LEDLGT_MIN,LEDLGT_MAX);
    ui->sliderledlgt->setPageStep(LEDLGT_STEP);

    m_timer = new QTimer();
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), SLOT(clearClickCount()));
    m_count = 0;

    loadWifiConfig();

    m_pUpdateTimer = new QTimer;
    m_pUpdateTimer->setInterval(1000);
    m_pUpdateTimer->start();
    connect(m_pUpdateTimer,SIGNAL(timeout()), SLOT(updateCurrentTime()));

}

VSSlideMenu::~VSSlideMenu()
{
    if(m_timer)
    {
        delete m_timer;
    }
    delete ui;
}

void VSSlideMenu::setSlidebacklgt(int value)
{
    ui->sliderbcklgt->setValue(value);
}

void VSSlideMenu::setSlideledlgt(int value)
{
    ui->sliderledlgt->setValue(value);
}

int VSSlideMenu::getLedLight()
{
    return ui->sliderledlgt->value();
}

void VSSlideMenu::setSwitchIcon()
{
    if(m_bWifi)
    {
        ui->button_switch->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
        ui->labelwifi->setStyleSheet(QString("border-image: url(:/res/icons/ic_wifi_1.png);"));
    }
    else
    {
        ui->button_switch->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
        ui->labelwifi->setStyleSheet(QString("border-image: url(:/res/icons/ic_wifi_3.png);"));
    }
}

void VSSlideMenu::loadWifiConfig()
{
    QVariant bWifi;
    if(ConfigInstance->readSet(ITEM_WIFI_ENABLE,bWifi))
    {
        if(0 == bWifi.toInt())
        {
            m_bWifi = true;
        }
        else
        {
            m_bWifi = false;
        }
        setSwitchIcon();
    }
}

/*状态改变事件处理函数(语言改变事件处理在这里)*/
void VSSlideMenu::changeEvent(QEvent*event)
{
    switch (event->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        QWidget::changeEvent(event);
        break;
    }
}

void VSSlideMenu::mousePressEvent(QMouseEvent *event)
{
    if(event->pos().x() < 0 || event->pos().y() < 0)
    {
        return;
    }

    if(event->pos().x() < ui->label_datetime->geometry().width()
        && event->pos().y() < ui->label_datetime->geometry().height())
    {
        emit sigDateTimeConfig();
    }

    QWidget::mousePressEvent(event);
}

void VSSlideMenu::updateCurrentTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm");
    if(str != ui->label_datetime->text())
    {
        ui->label_datetime->setText(str);
    }
}

void VSSlideMenu::on_button_option_clicked()
{
    if(m_timer->isActive())
        m_timer->stop();

    m_timer->start();

    m_count++;

    if(5 == m_count)
    {
        m_count = 0;
        emit sigOptionPage();
    }
}

void VSSlideMenu::clearClickCount()
{
    m_count = 0;
}

void VSSlideMenu::on_sliderbcklgt_valueChanged(int value)
{
    emit sigBackLight(value);
}

void VSSlideMenu::on_sliderledlgt_valueChanged(int value)
{
    emit sigLedLight(value);
}

void VSSlideMenu::on_button_switch_clicked()
{
    m_bWifi = !m_bWifi;
    if(true == m_bWifi)
    {
        if(ConfigInstance->writeSet(ITEM_WIFI_ENABLE,(QVariant)0))
        {
            setSwitchIcon();
            emit sigWifiEnable(true);
        }
    }
    else
    {
        if(ConfigInstance->writeSet(ITEM_WIFI_ENABLE,(QVariant)1))
        {
            setSwitchIcon();
            emit sigWifiEnable(false);
        }
    }
}
