#ifndef POWERSUPPLYPROCESS_H
#define POWERSUPPLYPROCESS_H

#include <QObject>
#include <QThread>
#include "thread.h"

#define UEVENT_MSG_LEN 4096

//电量
enum  EnumCapacity {
    Capacity_0,  //10
    Capacity_20_Red, //10-20
    Capacity_20, //20-40
    Capacity_40, //40-60
    Capacity_60, //60-80
    Capacity_80, //80-95
    Capacity_100, //95-100
};

enum EnumState {
    State_Discharging, //放电
    State_Charging, //充电
};

struct lutherGliethttp {
    const char *action;
    const char *path;
    const char *subsystem;
    const char *firmware;
    const char *powerstate;   //电池充电状态
    const char *powercapacity; //电量
    int maj;
    int min;
};

class PowerSupplyProcess : public QObject
{
    Q_OBJECT
public:
    explicit PowerSupplyProcess(QObject *parent = 0);

    void msgProcess(); //主回调函数，从下位机传递的输入事件中获取电池的相关信息

    void setPowerState(EnumState state) //设置充电状态
    {
        m_state = state;
    }

    void setPowerCapacity(EnumCapacity capacity) //设置电池电量等级
    {
        m_capacity = capacity;
    }

signals:
    void sigPowerState(EnumState state); //充电状态状态发生改变的信号

    void sigPowerCapacity(EnumCapacity capacity); //电量发生改变的信号

public slots:

private:
    int  openNetLinkSocket(); //NetLink套接字，用于实现中断和用户态进程间的通信

    void parseEvent(char *msg); //处理下位机传递的输入事件

private:
    Thread<PowerSupplyProcess> m_thread;  //主回调函数所在的次线程

    struct lutherGliethttp  m_lGliethttp; //用于处理输入事件的结构体

    EnumCapacity   m_capacity;

    EnumState m_state;

    int     m_devicefd;

};

#endif // POWERSUPPLYPROCESS_H
