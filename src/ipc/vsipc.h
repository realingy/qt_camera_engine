#ifndef VSIPC_H
#define VSIPC_H

#include <QObject>

class VSIpcClient : public QObject
{
    Q_OBJECT
public:
    VSIpcClient(QObject *parent=NULL) : QObject(parent), m_bConnect(false) {}
    virtual QByteArray readData() = 0;
    virtual quint64 writeData(QByteArray data) = 0;
    bool isConnect(){ return m_bConnect; }

signals:
    void receivedData(QByteArray);
    void IPCError(QString);

public slots:
    virtual int initConnect() = 0;
    virtual void handleDisconnect() = 0;

protected:
    void setIPCConnect(bool connect)
    {
        m_bConnect = connect;
    }

protected:
    bool m_bConnect;

};

#endif // VSIPC_H
