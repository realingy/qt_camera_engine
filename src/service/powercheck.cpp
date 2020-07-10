#include "powercheck.h"
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <QProcess>

PowerCheck::PowerCheck(QObject *parent)
    : QObject(parent)
    , m_powerCheckThread(this, &PowerCheck::powerCheck)
    , m_showPowerDialog(false)
    , m_poweroffForce(false)
{
    m_curKeyValue = KEY_RELEASE;
    m_keyCheckCount = 0;
    m_powerCheckThread.start();
    if(!getStanby()) {
        m_standby = false;
    } else {
        m_standby = true;
    }
}

void PowerCheck::powerCheck()
{
    int keyFd;
    struct input_event keyEvent;
    int key_code;

    keyFd = open(POWER_KEY_PATH, O_RDONLY|O_NONBLOCK);
    if(keyFd <= 0)
    {
        printf("open key error!\n");
        return;
    }

    while(!m_powerCheckThread.isStopped())
    {
        /*检测按键值*/
        if(read(keyFd, &keyEvent, sizeof(keyEvent)) == sizeof(keyEvent))
        {
            if(keyEvent.type != EV_KEY)
            {
                continue;
            }

            key_code = (int)keyEvent.code;
            m_curKeyValue = keyEvent.value;

            if(m_curKeyValue == KEY_RELEASE && 116 == key_code)//如果按键释放
            {
                if(true == m_poweroffForce)
                {
                    m_poweroffForce = false;
                }
                else if(true == m_showPowerDialog)
                {
                    m_showPowerDialog = false;
                }
                else
                {
                    qDebug() << "stand by!";
                    m_standby = !m_standby;
                    processStanby();
                }
                qDebug("key release!");
            }
        }

        /*检测 按键被按下的时间(计数),根据时间(计数)更改短按长按标志*/
        if(m_curKeyValue == KEY_PRESS && 116 == key_code)//如果按键是按下状态
        {
            m_keyCheckCount++;
            if(m_keyCheckCount < INTERVAL_SHOW_DIALOG)
            {
                //do nothing
            }
            else if(INTERVAL_SHOW_DIALOG <= m_keyCheckCount
                     && m_keyCheckCount < INTERVAL_POWEROFF_FORCE)
            {
                if(!m_showPowerDialog)
                {
                    qDebug("show power off dialog");
                    emit showPowerDialog();
                    m_showPowerDialog = true;
                }
            }
            else if(m_keyCheckCount >= INTERVAL_POWEROFF_FORCE)
            {
                if(!m_poweroffForce)
                {
                    qDebug() << "power off force!";
                    emit powerOffForce();
                    m_poweroffForce = true;
                }
            }
        }
        else
        {
            m_keyCheckCount = 0;
        }

        m_powerCheckThread.msleep(100);
    }

    close(keyFd);
}

void PowerCheck::processStanby()
{
    if(true == m_standby)
    {
        QString cmd = QString("echo 1 > /sys/class/graphics/fb0/blank");
        system(cmd.toLatin1().data());
    }
    else
    {
        QString cmd = QString("echo 0 > /sys/class/graphics/fb0/blank");
        system(cmd.toLatin1().data());
    }
}

int PowerCheck::getStanby()
{
    QProcess cmd;
    QStringList arg;
    arg.append(QString("echo 1 > /sys/class/graphics/fb0/blank"));
    cmd.start(QString("cat"), arg);
    cmd.waitForReadyRead();
    cmd.waitForFinished();
    QString standby(cmd.readAll());
    return standby.toInt();
}
