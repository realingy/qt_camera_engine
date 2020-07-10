#include "lightcontroller.h"
#include <QProcess>

LightController *LightController::m_instance = NULL;
QMutex LightController::m_mutex;

LightController::LightController()
{
    m_instance = this;
}

LightController *LightController::instance()
{
    if(NULL == m_instance)
    {
        m_mutex.lock();
        if(NULL == m_instance)
        {
            m_instance = new LightController();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

qint32 LightController::getBackLight()
{
#if 0
    QProcess cmd;
    cmd.start("cat",QString("/sys/class/backlight/backlight.17/brightness"));
    cmd.waitForReadyRead();
    cmd.waitForFinished();
    QString light(cmd.readAll());
    return light.toInt();
#endif
    return 0;
}

qint32 LightController::getLedLight()
{
    return 0;
}

void LightController::setBackLight(qint32 lightness)
{
    qint32 brightness;
    brightness = lightness*248/10;
    QString cmd = QString("echo %1 > /sys/class/backlight/pwm-backlight.0/brightness").arg(brightness);
    system(cmd.toLatin1().data());
}

void LightController::setLedLight(qint32 lightness)
{
    qint32 brightness;
    brightness = lightness*248/10;
    QString cmd = QString("echo %1 > /sys/devices/platform/pwm-backlight.1/backlight/pwm-backlight.1/brightness").arg(brightness);
    system(cmd.toLatin1().data());
}
