#ifndef KEYPRESSCHECK_H
#define KEYPRESSCHECK_H

#include <QObject>
#include "thread.h"
#include <QDebug>

#define KEY_PATH    "/dev/input/event0"
#define KEY_PRESS   1
#define KEY_RELEASE 0
#define MAX_SHORTPRESSCHECK_COUNT 3

/*按键检测是一个过程*/
class KeyPressCheck : public QObject
{
    Q_OBJECT
public:
    explicit KeyPressCheck(QObject *parent=0);
    ~KeyPressCheck();

    void start();//启动线程
    void stop();//停止线程

signals:
    void sigShortKeyPress(bool);
    void sigLongKeyPress(bool);

private:
    int m_keyFd;
    Thread<KeyPressCheck> m_keyPressCheckThread;
    unsigned int m_curKeyValue;
    unsigned int m_keyCheckCount;
    bool m_keyShortPressFlag;
    bool m_keyLongPressFlag;

private:
    void keyPressCheckThread();//按键检测线程函数
};

#endif // KEYPRESSCHECK_H
