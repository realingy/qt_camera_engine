#include "vsoptionpage.h"
#include "ui_vsoptionpage.h"
#include "vsglobal.h"
#include "statfs.h"
#include "ipcmanager.h"
#include "configmanager.h"
#include "version.h"
#include <QDate>
#include <QTime>
#include <QTimer>

/*!
 * \brief  ItemFrame::ItemFrame
 * \author 5005965
 * \date   2018.7.26
 */
ItemFrame::ItemFrame(QWidget *parent)
    : QFrame(parent)
    , m_scroll(false)
    , m_press(false)
{
    setStyleSheet(QString("background-color: rgb(255,255,255)"));
}

ItemFrame::~ItemFrame()
{
}

void ItemFrame::mousePressEvent(QMouseEvent *event)
{
    m_press = true;
    setStyleSheet(QString("background-color: rgb(193,193,193)"));
    QFrame::mousePressEvent(event);
}

void ItemFrame::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_press)
        return;

    m_scroll = true;

    QFrame::mouseMoveEvent(event);
}

void ItemFrame::mouseReleaseEvent(QMouseEvent *event)
{
    m_press = false;

    setStyleSheet(QString("background-color: rgb(255,255,255)"));

    if(!m_scroll)
    {
        emit itemClicked();
    }
    else
    {
        m_scroll = false;
    }

    QFrame::mouseReleaseEvent(event);
}

#define FACTOR (1024.0*1024.0*1024.0)

/*!
 * \brief  VSOptionPage::VSOptionPage
 * 配置页面类
 * \author 5005965
 * \date   2018.7.26
 */
VSOptionPage::VSOptionPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VSOptionPage)
    , m_bHardbutton(false)
    , m_bLogo(false)
    , m_triProcess(NULL)
{
    ui->setupUi(this);

    connect(ui->frame1_serial, SIGNAL(itemClicked()), SIGNAL(sigSerialSet())); //设置序列号
    connect(ui->frame5_name, SIGNAL(itemClicked()), SIGNAL(sigNameSet())); //设置设备名
    connect(ui->frame6_lang, SIGNAL(itemClicked()), SIGNAL(sigLanguageSet())); //设置语言
    connect(ui->frame7_standby, SIGNAL(itemClicked()), SIGNAL(sigStandbySet())); //设置自动待机
    connect(ui->frame8_auto_poweroff, SIGNAL(itemClicked()), SIGNAL(sigPowerOffSet())); //设置自动关机
    connect(ui->frame14_restore, SIGNAL(itemClicked()), SIGNAL(restoreFactory())); //恢复出厂设置

    ui->bar_storage->setTextVisible(false);
    ui->bar_storage->setRange(0,100);

    qRegisterMetaType<EnumDirection>("EnumDirection"); //注册三轴方向枚举元类型

    ui->label_version->setText(VERSION); //软件版本

    //获取服务器版本并显示
    connect(IpcInstance, SIGNAL(serverVersion(QString)), SLOT(setServerVersion(QString)));
    if(IpcInstance->connectToServer() < 0)
        ui->label_version_server->setText(tr("Server connect Error!"));
    else
        IpcInstance->getServerVersion();

    //创建三轴传感器处理类并连接
    m_triProcess = new TriaxialProcess;
    connect(m_triProcess, SIGNAL(sigTriaxial(EnumDirection,qlonglong)), SLOT(updateTriaxial(EnumDirection,qlonglong)), Qt::QueuedConnection);

    //计算硬盘容量并显示
    qlonglong total, avail;
    Statfs->calDiskInfo(total, avail);
    QString diskTotal = QString::number(total/FACTOR, 'f', 1);
    QString diskAvail = QString::number(avail/FACTOR, 'f', 1);

    ui->label_disk->setText(tr("total:%1 GB, avail:%2 GB").arg(diskTotal).arg(diskAvail));
    qlonglong fact = (float(total - avail)/total)*100;
    ui->bar_storage->setValue(fact);

    QTimer::singleShot(100, this, SLOT(initConfig()));
}

VSOptionPage::~VSOptionPage()
{
    delete ui;
}

void VSOptionPage::changeEvent(QEvent*event)
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

void VSOptionPage::setServerVersion(QString version)
{
    ui->label_version_server->setText(version);
}

void VSOptionPage::updateTriaxial(EnumDirection direction, qlonglong value)
{
    //将三轴传感器处理类发送的信息添加到队列中供处理
    switch (direction)
    {
        case 0:
            ui->tri_x->setText(QString::number(value));
            if(m_listX.count() == LISTLEN)
                m_listX.removeFirst();
            m_listX.append(value);
            break;
        case 1:
            ui->tri_y->setText(QString::number(value));
            if(m_listY.count() == LISTLEN)
                m_listY.removeFirst();
            m_listY.append(value);
            break;
        case 2:
            ui->tri_z->setText(QString::number(value));
            if(m_listZ.count() == LISTLEN)
                m_listZ.removeFirst();
            m_listZ.append(value);
            break;
        default:
            break;
    }
}

void VSOptionPage::on_button_restore_clicked()
{
    emit restoreFactory();
}

void VSOptionPage::on_button_lan_clicked()
{
    emit sigLanguageSet();
}

void VSOptionPage::on_button_name_clicked()
{
    emit sigNameSet();
}

void VSOptionPage::on_button_standby_clicked()
{
    emit sigStandbySet();
}

void VSOptionPage::on_button_power_clicked()
{
    emit sigPowerOffSet();
}

void VSOptionPage::on_button_hard_clicked()
{
    emit sigHardButtonSet();
}

//读取数据库中各项设置
void VSOptionPage::loadConfig()
{
    readLanConfig();
    readAutoPoweroffConfig();
    readAutoStandbyConfig();
    readHardbuttonConfig();
    readLogoConfig();
    readTimeFormatConfig();
    readNameConfig();
    readSerialConfig();
}

void VSOptionPage::readLanConfig()
{
    QVariant languge;
    if(ConfigInstance->readSet(ITEM_LAN,languge))
    {
        if(0 == languge.toInt())
        {
            ui->label_lan->setText(tr("Chinese"));
        }
        else
        {
            ui->label_lan->setText(QString("English"));
        }
    }
}

void VSOptionPage::readAutoPoweroffConfig()
{
    QVariant poweroff;
    if(ConfigInstance->readSet(ITEM_AUTOPOWEROFF,poweroff))
    {
        if(poweroff.toInt() > 0)
        {
            ui->label_power->setText(QString::number(poweroff.toInt()).append(QString(" min")));
        }
        else
        {
            ui->label_power->setText(QString("close"));
        }
    }
}

void VSOptionPage::readAutoStandbyConfig()
{
    QVariant standby;
    if(ConfigInstance->readSet(ITEM_AUTOSTANDBY,standby))
    {
        if(standby.toInt() > 0)
        {
            ui->label_standby->setText(QString::number(standby.toInt()).append(QString(" min")));
        }
        else
        {
            ui->label_standby->setText(QString("close"));
        }
    }
}

void VSOptionPage::readHardbuttonConfig()
{
    QVariant hardbutton;
    if(ConfigInstance->readSet(ITEM_HARDBUTTON_ENABLE,hardbutton))
    {
        if(0 == hardbutton.toInt())
        {
            m_bHardbutton = true;
            ui->switch_hardbutton->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
            ui->button_hard->setEnabled(true);
            ui->button_hard->show();
        }
        else if(1 == hardbutton.toInt())
        {
            m_bHardbutton = false;
            ui->switch_hardbutton->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
            ui->button_hard->setEnabled(false);
            ui->button_hard->hide();
        }
    }
}

void VSOptionPage::readLogoConfig()
{
    QVariant logo;
    if(ConfigInstance->readSet(ITEM_LOGO_ENABLE,logo))
    {
        if(0 == logo.toInt())
        {
            m_bLogo = true;
            ui->switch_logo->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
        }
        else if(1 == logo.toInt())
        {
            m_bLogo = false;
            ui->switch_logo->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
        }
    }
}

void VSOptionPage::readTimeFormatConfig()
{
    QVariant b24hformat;
    if(ConfigInstance->readSet(ITEM_TIME_FORMAT,b24hformat))
    {
        if(0 == b24hformat.toInt())
        {
            m_b24H = true;
            ui->button_24h->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
        }
        else if(1 == b24hformat.toInt())
        {
            m_b24H = false;
            ui->button_24h->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
        }
    }
}

void VSOptionPage::readNameConfig()
{
    QVariant name;
    if(ConfigInstance->readSet(ITEM_DEVICE_NAME,name))
    {
        ui->label_name->setText(name.toString());
    }
}

void VSOptionPage::readSerialConfig()
{
    QVariant serial;
    if(ConfigInstance->readSet(ITEM_DEVICE_SERIAL,serial))
    {
        ui->label_serial->setText(serial.toString());
    }
}

void VSOptionPage::on_switch_hardbutton_clicked()
{
    m_bHardbutton = !m_bHardbutton;
    if(true == m_bHardbutton)
    {
        if(ConfigInstance->writeSet(ITEM_HARDBUTTON_ENABLE,(QVariant)0))
        {
            ui->switch_hardbutton->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
            ui->button_hard->setEnabled(true);
            ui->button_hard->show();
        }
    }
    else
    {
        if(ConfigInstance->writeSet(ITEM_HARDBUTTON_ENABLE,(QVariant)1))
        {
            ui->switch_hardbutton->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
            ui->button_hard->setEnabled(false);
            ui->button_hard->hide();
        }
    }
}

void VSOptionPage::on_switch_logo_clicked()
{
    m_bLogo = !m_bLogo;
    if(true == m_bLogo)
    {
        if(ConfigInstance->writeSet(ITEM_LOGO_ENABLE,(QVariant)0))
        {
            ui->switch_logo->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
        }
    }
    else
    {
        if(ConfigInstance->writeSet(ITEM_LOGO_ENABLE,(QVariant)1))
        {
            ui->switch_logo->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
        }
    }
}

void VSOptionPage::on_button_serial_clicked()
{
    emit sigSerialSet();
}

void VSOptionPage::on_button_24h_clicked()
{
    m_b24H = !m_b24H;
    if(true == m_b24H)
    {
        if(ConfigInstance->writeSet(ITEM_TIME_FORMAT,(QVariant)0))
        {
            ui->button_24h->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_on.png);"));
        }
    }
    else
    {
        if(ConfigInstance->writeSet(ITEM_TIME_FORMAT,(QVariant)1))
        {
            ui->button_24h->setStyleSheet(QString("border-image: url(:/res/icons/ic_switch_off.png);"));
        }
    }
}

void VSOptionPage::initConfig()
{
    loadConfig(); //读取其他设置
}

void VSOptionPage::deviceMotionless()
{
#if 0
    //前90个数据的平均值和后10个平均值对比，获取设备的移动状态
    QList<qlonglong>::const_iterator it = m_listX.constBegin();

    qlonglong sum1 = 0;
    for(int i = 0; i < 90 && it != m_listX.constEnd(); i++)
    {
        sum1 += *it;
        it++;
    }
    int average1 = sum1 / 90;

    qlonglong sum2 = 0;
    while(it != m_listX.constEnd())
    {
        sum2 += *it;
        it++;
    }
    int average2 = sum2 / 10;

    if(abs(average1 - average2) < 500)
    {
//        emit sigMotionless();
    }
#endif
}
