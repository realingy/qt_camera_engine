#ifndef POWERCHECK_H
#define POWERCHECK_H

#include <QObject>
#include <QDebug>
#include "thread.h"

#define POWER_KEY_PATH    "/dev/input/event0"
#define KEY_PRESS   1
#define KEY_RELEASE 0
#define INTERVAL_SHOW_DIALOG 20
#define INTERVAL_POWEROFF_FORCE 50

/*按键检测是一个过程*/
class PowerCheck : public QObject
{
    Q_OBJECT
public:
    explicit PowerCheck(QObject *parent=0);

signals:
    void showPowerDialog();         //show poweroff dialog
    void powerOffForce();           //poweroff force

private:
    Thread<PowerCheck> m_powerCheckThread;

    unsigned int m_curKeyValue;
    unsigned int m_keyCheckCount;

    bool m_showPowerDialog;
    bool m_poweroffForce;
    bool m_standby;

private:
    void powerCheck();
    void processStanby();
    int getStanby();

};

#endif // POWERCHECK_H
