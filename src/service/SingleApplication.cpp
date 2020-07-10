#include "SingleApplication.h"
#include "configmanager.h"
#include <QtNetwork/QLocalSocket>
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>
#include <QProcess>

#define TIME_OUT    (100)    // 单位 ms

SingleApplication::SingleApplication(int &argc, char **argv, QString name)
    : QApplication(argc, argv)
    , m_IsRunning(false)
    , m_LocalServer(NULL)
    , m_ServerName(name)
    , m_autoPoweroffTimer(NULL)
    , m_autoStandbyTimer(NULL)
    , m_standby(false)
{
    InitLocalConnection();//取应用程序名作为LocalServer的名字

    m_autoPoweroffTimer = new QTimer;
    m_autoPoweroffTimer->setSingleShot(true);
    //connect(m_autoPoweroffTimer, SIGNAL(timeout()), SLOT(autoPoweroff()));

    QVariant interval_poweroff;
    if(ConfigInstance->readSet(ITEM_AUTOPOWEROFF,interval_poweroff))
    {
        if(interval_poweroff.toInt() > 0)
        {
            m_poweroffEnable = true;
            m_autoPowerOffInterval = interval_poweroff.toInt()*1000*60;
            m_autoPoweroffTimer->setInterval(m_autoPowerOffInterval);
            m_autoPoweroffTimer->start();
            qDebug() << "auto poweroff interval: " << interval_poweroff.toInt() << " min";
        }
        else
        {
            m_poweroffEnable = false;
        }
    }
    connect(ConfigInstance, SIGNAL(setPowerOffInterval()), SLOT(setPowerOffInterval()));

    m_autoStandbyTimer = new QTimer;
    m_autoStandbyTimer->setSingleShot(true);
    //connect(m_autoStandbyTimer, SIGNAL(timeout()), SLOT(autoStandby()));

    QVariant interval_standby;
    if(ConfigInstance->readSet(ITEM_AUTOSTANDBY, interval_standby))
    {
        if(interval_standby.toInt() > 0)
        {
            m_standbyEnable = true;
            m_autoStandbyInterval = interval_standby.toInt()*1000*60;
            m_autoStandbyTimer->setInterval(m_autoStandbyInterval);
            m_autoStandbyTimer->start();
            qDebug() << "auto standby interval: " << interval_standby.toInt() << " min";
        }
        else
        {
            m_standbyEnable = false;
        }
    }
    connect(ConfigInstance, SIGNAL(setStandbyInterval()), SLOT(setStandbyInterval()));
}

// 检查是否已經有一个实例在运行, true - 有实例运行， false - 没有实例运行
bool SingleApplication::isRunning()
{
    return m_IsRunning;
}

bool SingleApplication::notify(QObject *obj, QEvent *event)
{
//    Q_UNUSED(obj)
    if(QEvent::MouseButtonPress ==  event->type())
    {
        if(m_autoPoweroffTimer && true == m_poweroffEnable)
        {
            m_autoPoweroffTimer->stop();
        }
        if(m_autoStandbyTimer && true == m_standbyEnable)
        {
            m_autoStandbyTimer->stop();
            if(true == m_standby)
            {
                wakeDisplay();
            }
        }
    }
    else if(QEvent::MouseButtonRelease ==  event->type())
    {
        if(m_autoPoweroffTimer && true == m_poweroffEnable)
        {
            m_autoPoweroffTimer->start();
        }
        if(m_autoStandbyTimer && true == m_standbyEnable)
        {
            m_autoStandbyTimer->start();
        }
    }

    return QApplication::notify(obj, event);
}

// 通过socket通讯实现程序单实例运行，监听到新的连接时触发该函数
void SingleApplication::HasNewLocalConnection()
{
    QLocalSocket *socket = m_LocalServer->nextPendingConnection();
    if(socket)
    {
        socket->waitForReadyRead(2*TIME_OUT);
        delete socket;
    }
}

void SingleApplication::autoPoweroff()
{
    qDebug() << "time out, machine is going poweroff!";
//    QProcess::execute("poweroff");
}

void SingleApplication::setPowerOffInterval()
{
    QVariant interval_poweroff;
    if(ConfigInstance->readSet(ITEM_AUTOPOWEROFF,interval_poweroff))
    {
        if(interval_poweroff.toInt() > 0)
        {
            m_poweroffEnable = true;
            m_autoPowerOffInterval = interval_poweroff.toInt()*1000*60;
            m_autoPoweroffTimer->setInterval(m_autoPowerOffInterval);
            qDebug() << "auto poweroff interval: " << interval_poweroff.toInt() << " min";
        }
        else
        {
            m_poweroffEnable = false;
        }
    }
}

void SingleApplication::autoStandby()
{
    m_standby = true;
    qDebug() << "time out, machine is going standby!";
//    QString cmd = QString("echo 1 > /sys/class/graphics/fb0/blank");
//    system(cmd.toLatin1().data());
}

void SingleApplication::wakeDisplay()
{
    m_standby = false;
//    QString cmd = QString("echo 0 > /sys/class/graphics/fb0/blank");
//    system(cmd.toLatin1().data());
}

void SingleApplication::setStandbyInterval()
{
    QVariant interval_standby;
    if(ConfigInstance->readSet(ITEM_AUTOSTANDBY, interval_standby))
    {
        if(interval_standby.toInt() > 0)
        {
            m_standbyEnable = true;
            m_autoStandbyInterval = interval_standby.toInt()*1000*60;
            m_autoStandbyTimer->setInterval(m_autoStandbyInterval);
            qDebug() << "auto standby interval: " << interval_standby.toInt() << " min";
        }
        else
        {
            m_standbyEnable = false;
        }
    }
}

/*通过socket通讯实现程序单实例运行，初始化本地连接。如果连接不上server，则创建；否则退出*/
void SingleApplication::InitLocalConnection()
{
    m_IsRunning = false;

    QLocalSocket socket;
    socket.connectToServer(m_ServerName);
    qDebug() << " server: "<<m_ServerName;
    if(socket.waitForConnected(TIME_OUT))
    {
        fprintf(stderr, "%s already running.\n",m_ServerName.toLocal8Bit().constData());
        m_IsRunning = true;
        return;
    }
    else
    {
        CreateNewLocalServer();//连接不上服务器，就创建一个
    }
}

// 创建LocalServer
void SingleApplication::CreateNewLocalServer()
{
    m_LocalServer = new QLocalServer(this);
    connect(m_LocalServer, SIGNAL(newConnection()), this, SLOT(HasNewLocalConnection()));
    if(!m_LocalServer->listen(m_ServerName))
    {
        /*此时监听失败，可能是程序崩溃时,残留进程服务导致的,移除之*/
        if(m_LocalServer->serverError() == QAbstractSocket::AddressInUseError)
        {
            QLocalServer::removeServer(m_ServerName); // <-- 重点
            m_LocalServer->listen(m_ServerName); // 再次监听
        }
    }
}
