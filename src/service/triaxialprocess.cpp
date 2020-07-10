#include "triaxialprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <errno.h>
#include <QDebug>
#include <QDateTime>

struct input_event m_event;

TriaxialProcess::TriaxialProcess(QObject *parent)
    : QObject(parent)
    , m_thread(this, &TriaxialProcess::eventProc)
    , m_fd(0)
    , m_direction(DIR_X)
{
    QString cmd = QString("echo 1 > /sys/devices/virtual/input/input4/enable");
    system(cmd.toLatin1().data());
    m_thread.start();
}

void TriaxialProcess::eventProc()
{
    if((m_fd = open("/dev/input/event4",O_RDONLY,0)) < 0)
    {
        qDebug()<<"open /dev/input/event4 failed";
        return;
    }

    struct timeval tv;
    fd_set fds;

    while(!m_thread.isStopped())
    {
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 1000000;

        int ret = select(m_fd + 1, &fds, NULL, NULL, &tv);
        if(ret < 0)
        {
            if(EINTR == errno)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else if(ret == 0)
        {
            qDebug() << "timeout";
            continue;
        }
        else
        {
            int r = read(m_fd, &m_event, sizeof(m_event));
            if(r>0)
            {
                if(m_event.code==REL_X)
                {
                    m_direction = DIR_X;
                }
                else if(m_event.code==REL_Y)
                {
                    m_direction = DIR_Y;
                }
                else if(m_event.code==REL_Z)
                {
                    m_direction = DIR_Z;
                }
                else
                {
//                    qDebug() << "even code: " << m_event.code;
                }

                m_value = (qlonglong)m_event.value;
            }
        }

        if(m_value)
        {
            emit sigTriaxial(m_direction, m_value);
        }
        m_thread.msleep(50); //delay
    }
}
