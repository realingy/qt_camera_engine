#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include <QObject>
#include <QMutex>

#define LightInstance LightController::instance()

class LightController
{
public:
    LightController();

    static LightController *instance();

    qint32 getLedLight();

    qint32 getBackLight();

    void setLedLight(qint32 lightness);

    void setBackLight(qint32 lightness);

private:
    static LightController *m_instance;

    static QMutex m_mutex;

    qint8 m_ledLight;

    qint8 m_backLight;

};

#endif // LIGHTCONTROLLER_H
