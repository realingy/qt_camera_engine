#ifndef VSIPCTHREADCLIENT_H
#define VSIPCTHREADCLIENT_H

#include <QObject>
#include <QMutex>
#include "vsipc.h"
#include "vssocketclient.h"

#define IpcInstance IpcManager::instance()

class IpcManager : public QObject
{
    Q_OBJECT

public:
    IpcManager();
    ~IpcManager();

    int connectToServer();
    static IpcManager* instance();
    void takeAPicture(QString folder, QString fileName);
    void getServerVersion(); //获取服务器版本
    void startRecordVideo(QString folder, QString fileName); //开始录像
    void stopRecordVideo(); //停止录像
    int startPlayRecord(QString folder, QString fileName); //开始播放录像
    int pause(bool bPause); //暂停播放
    int stopPlayRecord(); //停止播放
    int skipTo(uint frameID);
    void disconnect(); //结束通信,回收资源

public slots:
    void handleReceiveData(QByteArray data);

signals:
    void newPicture(QString fileName); //拍照
    void serverVersion(QString version); //获取服务器版本
    void sigRecordAck(int statusRecord, QString file); //录像的应答
    void sigDisplayAck(int statusDisplay, int time); //播放的应答
    void sigSkipAck(int skipValue); //跳转的应答

private:
    void handlePacket(QByteArray data);
    void handleImageSubCmd(const char *ptr);
    void handleVideoSubCmd(const char *ptr);
    void handleServerVersion(const char *ptr);

private:
    static IpcManager *m_instance;
    static QMutex  m_mutex;
    VSSocketClient 	*m_ipc;
    QByteArray	m_data;

};

#endif // VSIPCTHREADCLIENT_H
