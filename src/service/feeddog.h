#ifndef FEEDDOG_H
#define FEEDDOG_H

#include <QLocalSocket>
#include <QThread>
#include <QMutex>

#define FEED_DOG_REQ 0x0001//请求
#define FEED_DOG_RSP 0x0002//应答

#define HEAD 0x68
#define TAIL 0x16

#define SERVER_NAME "WatchDog"
#define TIME_OUT 100

#define MAX_TIMEOUT_COUNT  10

typedef struct
{
    signed char Head;//帧头
    unsigned short Len;//数据长度
    char AppName[64];//应用名称
    unsigned short CmdCode;//命令码
    signed char Data;//数据
    signed char ChkSum;//校验和
    signed char Tail;//帧尾
}__attribute__((packed, aligned(1))) IPCDataStruct;

typedef struct
{
    bool * ThreadAliveVar;//线程存活变量
    int TimeoutCount;//超时次数
}ThreadMonitorStruct;

/*喂狗类:在新线程里，定时向看门狗发送信息*/
class FeedDog: public QThread
{
    Q_OBJECT
public:
    static FeedDog * instance();//单实例接口
    void RegistThreadForMonitor(bool & ThreadAliveVar);

protected:
    void run();

private:
    FeedDog(QThread * parent = 0);
    unsigned char CalcSumByte(const QByteArray &buf, unsigned short length);//计算校验和
    int PackIPCData(QByteArray &buf, IPCDataStruct IPCMsg);//打包IPC消息
    bool IsThreadsNormal();//线程是否存活

private:
    static FeedDog *m_instance;
    static QMutex  m_mutex;
    IPCDataStruct m_IPCData;
    std::vector<ThreadMonitorStruct> m_ThreadMonitorVector;
    QMutex m_ThreadMonitorMutex;
};

#endif // FEEDDOG_H
