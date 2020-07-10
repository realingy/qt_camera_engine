#ifndef VSSOCKETCLIENT_H
#define VSSOCKETCLIENT_H

#include <QLocalSocket>
#include <QObject>
#include "vsipc.h"

class VSSocketClient : public VSIpcClient
{
    Q_OBJECT

public:
    VSSocketClient(QObject *parent = 0);
    ~VSSocketClient();

    virtual QByteArray readData();
    virtual quint64 writeData(QByteArray data);

signals:
    void receivedData(QByteArray data);

public slots:
    virtual int initConnect();
    virtual void handleDisconnect();
    void handleError(QLocalSocket::LocalSocketError error);
    void handleReadyRead();

private:
    QLocalSocket *m_socket;

};

#endif // VSSOCKETCLIENT_H
