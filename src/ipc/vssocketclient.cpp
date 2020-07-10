#include "vssocketclient.h"
#include <QPushButton>

//#define SERVER_NAME "IPCServer"
#define SERVER_NAME "vs31_server"
#define TIME_OUT 100

VSSocketClient::VSSocketClient(QObject *parent)
    : VSIpcClient(parent)
    , m_socket(NULL)
{
}

VSSocketClient::~VSSocketClient()
{
//    handleDisconnect();
    delete m_socket;
    m_socket = NULL;
}

int VSSocketClient::initConnect()
{
    if(!m_socket)
    {
        m_socket = new QLocalSocket();
        connect(m_socket, SIGNAL(disconnected()), SLOT(handleDisconnect()));
        connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)),  SLOT(handleError(QLocalSocket::LocalSocketError)));
        connect(m_socket, SIGNAL(readyRead()), SLOT(handleReadyRead()));
    }
    if(m_socket->state() == QLocalSocket::ConnectedState)
        return 0;

    m_socket->connectToServer(SERVER_NAME);   //server name, server need listen it
    if(!m_socket->waitForConnected(TIME_OUT))
    {
        qDebug()<<"Server is Not Running";
        return -1;
    }
    qDebug() << "connected";

    setIPCConnect(true);
    return 0;
}

QByteArray VSSocketClient::readData()
{
    return m_socket->read(0xFF);
}

quint64 VSSocketClient::writeData(QByteArray data)
{
    if(m_socket && m_socket->state() == QLocalSocket::ConnectedState)
    {
        return m_socket->write(data);
    }
    else
    {
        return 0;
    }
}

void VSSocketClient::handleDisconnect()
{
    if(m_socket)
    {
        setIPCConnect(false);
        if(QLocalSocket::ConnectedState == m_socket->state())
        {
            m_socket->disconnectFromServer();
        }
//        delete m_socket;
//        m_socket = NULL;

        qDebug()<<"disconnected.";
    }
}

void VSSocketClient::handleError(QLocalSocket::LocalSocketError error)
{
    emit IPCError(m_socket->errorString());

    qWarning()<<error;
}

void VSSocketClient::handleReadyRead()
{
    emit receivedData(m_socket->readAll());
}
