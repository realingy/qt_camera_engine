#include "feeddog.h"
#include <QCoreApplication>
#include <QFileInfo>

FeedDog *FeedDog::m_instance = NULL;
QMutex  FeedDog::m_mutex;

FeedDog::FeedDog(QThread *parent):QThread(parent)
{
    /*初始化IPC结构体*/
    m_IPCData.Head = HEAD;//帧头
    m_IPCData.CmdCode = FEED_DOG_REQ;//命令码

    /*应用名称*/
    memset(m_IPCData.AppName, 0, sizeof(m_IPCData.AppName));
    QByteArray Temp = QFileInfo(QCoreApplication::applicationFilePath()).fileName().toLatin1();
    qDebug()<<QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    strcpy(m_IPCData.AppName, Temp.data());

    m_IPCData.Data = 0x00;//数据
    m_IPCData.Tail = TAIL;//帧尾

    m_ThreadMonitorVector.clear();
}

//单实例接口
FeedDog*FeedDog::instance()
{
    if(NULL == m_instance)
    {
        QMutexLocker locker(&m_mutex);
        if(NULL == m_instance)
        {
            m_instance = new FeedDog();
        }
    }
    return m_instance;
}

void FeedDog::RegistThreadForMonitor(bool &ThreadAliveVar)
{
    QMutexLocker locker(&m_ThreadMonitorMutex);

    ThreadMonitorStruct ThreadMonitor;
    ThreadMonitor.ThreadAliveVar = &ThreadAliveVar;
    ThreadMonitor.TimeoutCount = 0;
    m_ThreadMonitorVector.push_back(ThreadMonitor);
}


/*在新线程里，定时向看门狗发送信息*/
void FeedDog::run()
{
    QByteArray FeedDogMsg;

    QLocalSocket * Socket = new QLocalSocket();//创建本地套接字
    while(1)
    {
        if(IsThreadsNormal()==false)
            break;

        if(Socket->state() == QLocalSocket::ConnectedState)//如果状态是已经连接的，则立即向看门狗发送信息
        {
            PackIPCData(FeedDogMsg, m_IPCData);
            Socket->write(FeedDogMsg);
            Socket->flush();
            Socket->waitForBytesWritten();
            //qDebug()<<QDateTime::currentDateTime().toString("hh:mm:ss.zzz")<<" Client->write date";
        }
        else//否则，连接服务器
        {
            Socket->connectToServer(SERVER_NAME);
            if(!Socket->waitForConnected(TIME_OUT))
            {
                qDebug()<<"server connect to Watchdog failed";
                msleep(1000);
                continue;
            }
        }
        msleep(1000);//1000ms喂狗一次
    }
    Socket->close();//关闭本地套接字
    Socket->deleteLater();//删除套接字对象
}

//计算校验和——一个字节
unsigned char FeedDog::CalcSumByte(const QByteArray &buf, unsigned short length)
{
   unsigned char sum;
   unsigned short i;

   sum = 0;
   for(i=0 ;i<length ;i++)
   {
       sum += buf[i];
   }

   return sum;
}

//打包IPC消息
int FeedDog::PackIPCData(QByteArray &buf, IPCDataStruct IPCMsg)
{
    unsigned short Len = 0, i, TempPos;
    unsigned char ChkSum;

    buf.clear();
    buf[Len++] = IPCMsg.Head;//帧头

    /*数据长度，先跳过*/
    TempPos = Len;
    Len += 2;

    /*应用名称*/
    for(i=0; i<sizeof(IPCMsg.AppName); i++)
    {
        buf[Len+i] = IPCMsg.AppName[i];
    }
    Len += sizeof(IPCMsg.AppName);

    /*命令码*/
    buf[Len++] = IPCMsg.CmdCode&0xFF;
    buf[Len++] = IPCMsg.CmdCode>>8;

    buf[Len++] = IPCMsg.Data;//数据

    /*计算长度值，并填充*/
    IPCMsg.Len = Len - 3;
    buf[TempPos+0] = IPCMsg.Len&0xFF;
    buf[TempPos+1] = IPCMsg.Len>>8;

    /*校验和*/
    ChkSum = CalcSumByte(buf, Len);
    buf[Len++] = ChkSum;

    buf[Len++] = IPCMsg.Tail;//帧尾

    return Len;
}

//线程是否存活
bool FeedDog::IsThreadsNormal()
{
    QMutexLocker locker(&m_ThreadMonitorMutex);
    unsigned int i;

    for(i=0; i<m_ThreadMonitorVector.size(); i++)
    {
        if(!m_ThreadMonitorVector[i].ThreadAliveVar)//如果线程存活变量为false,则超时计数自加
        {
            m_ThreadMonitorVector[i].TimeoutCount++;
            if(m_ThreadMonitorVector[i].TimeoutCount >= MAX_TIMEOUT_COUNT)//如果超时变量超过最大允许值，则返回false
            {
                return false;
            }
        }
        else
        {
            m_ThreadMonitorVector[i].TimeoutCount = 0;
        }
        m_ThreadMonitorVector[i].ThreadAliveVar = false;
    }

    return true;
}
