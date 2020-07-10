#include "ipcmanager.h"
#include "vsprotocol.h"
#include "vsglobal.h"

#define SERVER_NAME "IPCServer"
#define TIME_OUT 100

IpcManager *IpcManager::m_instance = NULL;
QMutex  IpcManager::m_mutex;

IpcManager::IpcManager()
    : QObject()
    , m_ipc(NULL)
{
    m_data.clear();
    m_instance = this;
}

IpcManager::~IpcManager()
{
    delete m_ipc;
}

IpcManager* IpcManager::instance()
{
    if(NULL == m_instance)
    {
        m_mutex.lock();
        if(NULL == m_instance)
        {
            m_instance = new IpcManager();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

int IpcManager::connectToServer()
{
    if(!m_ipc)
    {
        m_ipc = new VSSocketClient();
        connect(m_ipc, SIGNAL(receivedData(QByteArray)), SLOT(handleReceiveData(QByteArray)));
    }
    if(m_ipc->isConnect())
        return 0;
    return m_ipc->initConnect();
}

void IpcManager::disconnect()
{
    if(m_ipc)
    {
        m_ipc->handleDisconnect();
        delete m_ipc;
        m_ipc = NULL;
    }
}

void IpcManager::handleReceiveData(QByteArray data)
{
    m_data.append(data);
    QByteArray head;
    head[0]=HEADER_1;
    head[1]=HEADER_2;
    int headSize=4;

    do{
        int index = m_data.indexOf(head);
        if(index<0)
            m_data.clear();
        m_data=m_data.remove(0,index);
        if(m_data.size() < MIN_PACKET_SIZE)
            return;
        unsigned char *pData=(unsigned char *)m_data.data();
        quint16 size = (pData[LENGTH_H_OFFSET] << 8) | pData[LENGTH_L_OFFSET];
        if(headSize+size+2 > m_data.size())
            return;

        quint16 newChecksum = qChecksum((const char *)(pData+headSize),size);
        quint16 packetChecksum = ((pData[headSize+size] << 8) | pData[headSize+size+1]);
        if(newChecksum == packetChecksum)
        {
            handlePacket(m_data.left(headSize+size));
            m_data.remove(0,headSize+size+2);
        }
    }while(1);
}

void IpcManager::handlePacket(QByteArray data)
{
    qDebug()<<"new packet"<<data.toHex();
    quint8 ID = data[4];
//    quint8 subCmdNumber = data[5];
    switch (ID)
    {
    case 0:
        handleImageSubCmd(data.data()+5);
        break;
    case 1:
        handleVideoSubCmd(data.data()+5);
        break;
    case 2:

        break;
    case 4:
        handleServerVersion(data.data()+5);
        break;
    default:
        break;
    }
}

void IpcManager::handleImageSubCmd(const char *ptr)
{
    quint8 sum = ptr[0];
    QString folder, name;
    bool takePhotoAck = false;
    ptr++;
    for(int i=0; i<sum; i++)
    {
        quint8 ID = ptr[0];
        quint32 size = ptr[1];
        if(0 == ID)
        {
            folder = QString(ptr+2).left(size);
        }
        else if(1 == ID)
        {
            name = QString(ptr+2).left(size);
        }
        else if(2 == ID)
        {
            char content = *(ptr+2);
            if(0 == (content & 1))
            {
                takePhotoAck = true;
            }
        }
        ptr += 2 + size;
    }
    if(true == takePhotoAck && MEDIA_FOLDER == folder)
    {
        emit newPicture(name);
    }
}

void IpcManager::handleVideoSubCmd(const char *ptr)
{
    quint8 sum = ptr[0];
    QString folder, name;
    int skipValue = -1;
    int statusRecord = -1;
    int statusDisplay = -1;
    int time = 0;
    bool success = true;
    ptr++;
    for(int i=0; i<sum; i++)
    {
        quint8 subId = ptr[0];
        quint32 size = ptr[1];

        switch (subId) {
        case 0:
            {
                char content = *(ptr+2);

                //record
                if((char)0 == content)
                    statusRecord = 1; //stop
                else if((char)2 == content)
                    statusRecord = 0; //start

                //display
                if((char)1 == content)
                    statusDisplay = 1; //stop
                else if((char)3 == content)
                    statusDisplay = 0; //start
                else if((char)5 == content)
                    statusDisplay = 2; //pause
            }
            break;
        case 1:
            folder = QString(ptr+2).left(size);
            break;
        case 2:
            name = QString(ptr+2).left(size);
            break;
        case 3:
            {
                char content = *(ptr+2);
                if(1 == (content & 1))
                    success = false;
            }
            break;
        case 4:
            skipValue = QString(ptr+2).left(size).toInt();
            break;
        case 5:
            time = QString(ptr+2).left(size).toInt();
            break;
        default:
            break;
        }
        ptr += 2 + size;
    }
    if(false == success)
    {
        return;
    }

    if(statusRecord >= 0 && MEDIA_FOLDER == folder)
    {
        emit sigRecordAck(statusRecord, folder+name);
    }
    else if(statusDisplay >= 0 && skipValue < 0)
    {
        emit sigDisplayAck(statusDisplay, time);
    }
    else if(skipValue >= 0)
    {
        emit sigSkipAck(skipValue);
    }
}

void IpcManager::handleServerVersion(const char *ptr)
{
    quint8 sum = ptr[0];
    QString version;
    ptr++;

    if(1 == sum)
    {
        quint8 ID = ptr[0];
        quint32 size = ptr[1];
        if(0 == ID)
        {
            version = QString(ptr+2).left(size);
        }
    }
    emit serverVersion(version);
}

void IpcManager::getServerVersion()
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd = VSProtocol::builder(4,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != cmd.size())
        qDebug() << "take photo cmd is written failed!";
    else
        qDebug() << "take photo cmd is written success!";
}

void IpcManager::takeAPicture(QString folder, QString fileName)
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(quint8)0;
    cmd[cmd.size()]=(quint8)folder.size();
    cmd.append(folder);
    list.append(cmd);

    cmd.clear();
    cmd[cmd.size()]=(quint8)1;
    cmd[cmd.size()]=(quint8)fileName.size();
    cmd.append(fileName);
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(0,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
        qDebug() << "take photo cmd is written failed!";
    else
        qDebug() << "take photo cmd is written success!";
}

void IpcManager::startRecordVideo(QString folder, QString fileName)
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(quint8)0;   //视频控制
    cmd[cmd.size()]=(quint8)1;
    cmd.append((quint8)2);       //开始录像 0x00000010 bit0: 0 bit1: 1 bit2: 0
    list.append(cmd);

    cmd.clear();
    cmd[cmd.size()]=(quint8)1;
    cmd[cmd.size()]=(quint8)folder.size();
    cmd.append(folder);
    list.append(cmd);

    cmd.clear();
    cmd[cmd.size()]=(quint8)2;
    cmd[cmd.size()]=(quint8)fileName.size();
    cmd.append(fileName);
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(1,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
    {
        qDebug() << "start record cmd is written failed!";
    }
    else
    {
        qDebug() << "start record cmd is written success!";
    }
}

void IpcManager::stopRecordVideo()
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(char)0;   //视频控制
    cmd[cmd.size()]=(char)1;
    cmd.append((char)0);       //停止录像 0x00000000 bit0: 0 bit1: 0 bit2: 0
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(1,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
    {
        qDebug() << "stop record cmd is written failed!";
    }
    else
    {
        qDebug() << "stop record cmd is written success!";
    }
}

int IpcManager::startPlayRecord(QString folder, QString fileName)
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(quint8)0;   //视频控制
    cmd[cmd.size()]=(quint8)1;
    cmd.append((quint8)3);       //开始播放录像 0x00000011 bit0: 1 bit1: 1 bit2: 0
    list.append(cmd);

    cmd.clear();
    cmd[cmd.size()]=(quint8)1;
    cmd[cmd.size()]=(quint8)folder.size();
    cmd.append(folder);
    list.append(cmd);

    cmd.clear();
    cmd[cmd.size()]=(quint8)2;
    cmd[cmd.size()]=(quint8)fileName.size();
    cmd.append(fileName);
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(1,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
    {
        qDebug() << "start play record cmd is written failed!";
    }
    else
    {
        qDebug() << "start play record cmd is written success!";
    }
}

int IpcManager::pause(bool bPause)
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(quint8)0;   //视频控制
    cmd[cmd.size()]=(quint8)1;
    cmd.append((quint8)5);       //开始播放录像 0x00000101 bit0: 1 bit1: 0 bit2: 1
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(1,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
    {
        qDebug() << "stop play record cmd is written failed!";
    }
    else
    {
        qDebug() << "stop play record cmd is written success!";
    }
}

int IpcManager::stopPlayRecord()
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(quint8)0;   //视频控制
    cmd[cmd.size()]=(quint8)1;
    cmd.append((quint8)1);       //开始播放录像 0x00000001 bit0: 1 bit1: 0 bit2: 0
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(1,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
    {
        qDebug() << "stop play record cmd is written failed!";
    }
    else
    {
        qDebug() << "stop play record cmd is written success!";
    }
}

int IpcManager::skipTo(uint frameID)
{
    QList<QByteArray> list;
    QByteArray cmd;
    list.clear();

    cmd.clear();
    cmd[cmd.size()]=(quint8)4;   //视频跳转
    cmd[cmd.size()]=(quint8)4;
    cmd.append((qlonglong)frameID);   //开始播放录像 0x00000001 bit0: 1 bit1: 0 bit2: 0
    list.append(cmd);

    cmd.clear();
    cmd = VSProtocol::builder(1,list);
    qDebug() << " new cmd " << cmd.toHex();
    if(m_ipc->writeData(cmd) != (quint64)cmd.size())
    {
        qDebug() << "video skip cmd is written failed!";
    }
    else
    {
        qDebug() << "video skip cmd is written success!";
    }
}
