#include "powersupplyprocess.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <QDebug>
#include <QProcess>

PowerSupplyProcess::PowerSupplyProcess(QObject *parent)
    : QObject(parent)
    , m_thread(this, &PowerSupplyProcess::msgProcess)
    , m_devicefd(-1)
{
    m_devicefd = openNetLinkSocket();
    m_thread.start();
}

//主回调函数，从下位机传递的输入事件中获取电池的相关信息
void PowerSupplyProcess::msgProcess()
{
    int len;
    char msg[UEVENT_MSG_LEN+2]; //为什么加两个空字符????
    do {
        while((len = recv(m_devicefd, msg, UEVENT_MSG_LEN, 0)) > 0)
        {
            if(len == UEVENT_MSG_LEN)
                continue;

            msg[len] = '\0';
            msg[len+1] = '\0';

            parseEvent(msg);
        }
    } while(!m_thread.isStopped());
}

//NetLink套接字，用于实现中断和用户态进程间的通信
int PowerSupplyProcess::openNetLinkSocket()
{
    struct sockaddr_nl addr;
    int sz = 64*1024; //为什么是64k
    int fd;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK; //固定
    addr.nl_pid = getpid(); //将当前的进程ID作为通信的标识
    addr.nl_groups = 0xffffffff; //多播组掩码,似乎也是固定的？

    fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT); //生成一个PF_NETLINK族套接字
    if (fd < 0)
        return -1;

    setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)); //设置套接字选项

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) //绑定netlink结构体
    {
        close(fd);
        return -1;
    }

    return fd;
}

//处理一条消息，从一条消息中获取有用的信息(电池相关的)
void PowerSupplyProcess::parseEvent(char *msg)
{
    if(!msg)
        return;
    //用于存储电池相关信息的结构体初始化
    m_lGliethttp.action = "";
    m_lGliethttp.path = "";
    m_lGliethttp.subsystem = "";
    m_lGliethttp.firmware = "";
    m_lGliethttp.powerstate = "";
    m_lGliethttp.powercapacity = "";
    m_lGliethttp.maj = -1;
    m_lGliethttp.min = -1;

    while(*msg)
    {
        qDebug() << "msg ======> " << msg;

        if(!strncmp(msg, "ACTION=", 7))
        {
            msg += 7;
            m_lGliethttp.action = msg;
        }
        else if(!strncmp(msg, "DEVPATH=", 8))
        {
            msg += 8;
            m_lGliethttp.path = msg;
        }
        else if(!strncmp(msg, "SUBSYSTEM=", 10))
        {
            msg += 10;
            m_lGliethttp.subsystem = msg;
        }
        else if(!strncmp(msg, "FIRMWARE=", 9))
        {
            msg += 9;
            m_lGliethttp.firmware= msg;
        }
        else if(!strncmp(msg, "POWER_SUPPLY_STATUS=", 20))
        {
            msg += 20;
            m_lGliethttp.powerstate = msg;
        }
        else if(!strncmp(msg, "POWER_SUPPLY_CAPACITY=", 22))
        {
            msg += 22;
            m_lGliethttp.powercapacity = msg;
        }
        else if(!strncmp(msg, "MAJOR=", 6))
        {
            msg += 6;
            m_lGliethttp.maj = atoi(msg);
        }
        else if(!strncmp(msg, "MINOR=", 6))
        {
            msg += 6;
            m_lGliethttp.min = atoi(msg);
        }

        while(*msg++)
            ;
    }

    qDebug() << m_lGliethttp.action;
    qDebug() << m_lGliethttp.path;
    qDebug() << m_lGliethttp.subsystem;
    qDebug() << m_lGliethttp.firmware;
    qDebug() << m_lGliethttp.powerstate;
    qDebug() << m_lGliethttp.powercapacity;
    qDebug() << m_lGliethttp.maj;
    qDebug() << m_lGliethttp.min;

    int capacity = QString(m_lGliethttp.powercapacity).toInt();

    if(capacity < 10 && m_capacity != Capacity_0)
    {
        m_capacity = Capacity_0;
        emit sigPowerCapacity(Capacity_0);
    }
    else if(capacity >= 10 && capacity < 20 && m_capacity != Capacity_20_Red)
    {
        m_capacity = Capacity_20_Red;
        emit sigPowerCapacity(Capacity_20_Red);
    }
    else if(capacity >= 20 && capacity < 40 && m_capacity != Capacity_20)
    {
        m_capacity = Capacity_20;
        emit sigPowerCapacity(Capacity_20);
    }
    else if(capacity >= 40 && capacity < 60 && m_capacity != Capacity_40)
    {
        m_capacity = Capacity_40;
        emit sigPowerCapacity(Capacity_40);
    }
    else if(capacity >= 60 && capacity < 80 && m_capacity != Capacity_60)
    {
        m_capacity = Capacity_60;
        emit sigPowerCapacity(Capacity_60);
    }
    else if(capacity >= 80 && capacity < 95 && m_capacity != Capacity_80)
    {
        m_capacity = Capacity_80;
        emit sigPowerCapacity(Capacity_80);
    }
    else if(capacity >= 95 && capacity < 100 && m_capacity != Capacity_100)
    {
        m_capacity = Capacity_100;
        emit sigPowerCapacity(Capacity_100);
    }

    if(m_lGliethttp.powerstate == QString("Discharging") || m_lGliethttp.powerstate == QString("Not charging"))
    {
        if(m_state == State_Charging)
        {
            m_state = State_Discharging;
            emit sigPowerState(State_Discharging);
        }
    }
    else if(m_lGliethttp.powerstate == QString("Charging"))
    {
        if(m_state == State_Discharging)
        {
            m_state = State_Charging;
            emit sigPowerState(State_Charging);
        }
    }

//    printf("event { '%s', '%s', '%s', '%s', %d, %d }\n",
//                    luther_gliethttp->action, luther_gliethttp->path, luther_gliethttp->subsystem,
//                    luther_gliethttp->firmware, luther_gliethttp->maj, luther_gliethttp->min);

}
