#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QtNetwork/QLocalServer>

class SingleApplication : public QApplication
{
    Q_OBJECT
public:
    SingleApplication(int &argc, char **argv, QString name);
    bool isRunning();// 是否已經有实例在运行

    bool notify(QObject *obj, QEvent *event);

private slots:
    void HasNewLocalConnection();// 有新连接时触发

    void autoPoweroff();

    void autoStandby();

public slots:
    void setPowerOffInterval();

    void setStandbyInterval();

private:
    void wakeDisplay();

    void InitLocalConnection();// 初始化本地连接

    void CreateNewLocalServer();// 创建服务端

private:
    bool m_IsRunning;                // 是否已經有实例在运行
    QLocalServer *m_LocalServer;     // 本地socket Server
    QString  m_ServerName;            // 服务名称

    QTimer  *m_autoPoweroffTimer;
    int      m_autoPowerOffInterval;

    QTimer  *m_autoStandbyTimer;
    int      m_autoStandbyInterval;

    bool     m_standby;
    bool     m_standbyEnable;
    bool     m_poweroffEnable;

};

#endif // SINGLEAPPLICATION_H
