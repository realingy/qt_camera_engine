#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include "keypresscheck.h"

KeyPressCheck::KeyPressCheck(QObject *parent) : QObject(parent),
    m_keyPressCheckThread(this, &KeyPressCheck::keyPressCheckThread)
{
    m_keyFd = open(KEY_PATH, O_RDONLY|O_NONBLOCK);
    if(m_keyFd <= 0)
    {
        printf("open key error!\n");
        return;
    }

    m_curKeyValue = KEY_RELEASE;
    m_keyCheckCount = 0;
    m_keyShortPressFlag = m_keyLongPressFlag = false;
}


KeyPressCheck::~KeyPressCheck()
{
    close(m_keyFd);
}

//启动线程
void KeyPressCheck::start()
{
    m_keyPressCheckThread.start();
}

//停止线程
void KeyPressCheck::stop()
{
    m_keyPressCheckThread.stop();
}

//按键检测线程函数
void KeyPressCheck::keyPressCheckThread()
{
    struct input_event keyEvent;
    int key_code;

    while(!m_keyPressCheckThread.isStopped())
    {
        /*检测按键值*/
        if(read(m_keyFd, &keyEvent, sizeof(keyEvent)) == sizeof(keyEvent))
        {
            if(keyEvent.type != EV_KEY)
            {
                continue;
            }

            key_code = (int)keyEvent.code;
            m_curKeyValue = keyEvent.value;

            if(m_curKeyValue == KEY_RELEASE && 115 == key_code)//如果按键释放
            {
                if(m_keyShortPressFlag)
                {
                    m_keyShortPressFlag = false;
                    emit sigShortKeyPress(m_keyShortPressFlag);
                    //qDebug()<<"key short press";
                }

                if(m_keyLongPressFlag)
                {
                    m_keyLongPressFlag = false;
                    emit sigLongKeyPress(false);
                    //qDebug()<<"key long press end";
                }
            }
        }

        /*检测 按键被按下的时间(计数),根据时间(计数)更改短按长按标志*/
        if(m_curKeyValue == KEY_PRESS && 115 == key_code)//如果按键是按下状态
        {
            m_keyCheckCount++;
            if(m_keyCheckCount <= MAX_SHORTPRESSCHECK_COUNT)
            {
                if(!m_keyShortPressFlag)
                {
                    m_keyShortPressFlag = true;
                }
            }
            else
            {
                m_keyShortPressFlag = false;//短按变长按，短按失效
                if(!m_keyLongPressFlag)
                {
                    m_keyLongPressFlag = true;
                    emit sigLongKeyPress(true);
                    //qDebug()<<"key long press start";
                }
                else
                {
                    m_keyCheckCount = MAX_SHORTPRESSCHECK_COUNT+1;
                }
            }
        }
        else
        {
            m_keyCheckCount = 0;
        }

        m_keyPressCheckThread.msleep(100);
    }
}
