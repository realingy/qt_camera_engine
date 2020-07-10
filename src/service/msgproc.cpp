#include "msgproc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

MsgProc::MsgProc()
    : m_fifo("")
{
}

MsgProc::~MsgProc()
{
    if(m_fifo.size())
        unlink(m_fifo.toLocal8Bit());
}

void MsgProc::slotUpgradeAccept()
{
    QString cmd = QString("echo \"#Sig=upgradeAccept\" > /tmp/app.fifo");
    system(cmd.toLatin1().data());
}

void MsgProc::slotUpgradeReject()
{
    QString cmd = QString("echo \"#Sig=upgradeReject\" > /tmp/app.fifo");
    system(cmd.toLatin1().data());
}

void MsgProc::run()
{
    char *fifo=(char *)"/tmp/app.fifo";
    m_fifo = fifo;
    if(access(fifo, R_OK|W_OK) == -1)
    {
        if(mkfifo(fifo, 0777) != 0)
        {
            qDebug() << "can not create fifo " << QString(fifo);
            exit(EXIT_FAILURE);
        }
    }

    FILE *fd = fopen(fifo, "r");
    if(!fd)
    {
        qDebug()<<"can not open fifo";
        exit(EXIT_FAILURE);
    }

    QTextStream stream(fd);
    while(1)
    {
        QString msg = stream.readLine();
        if(msg.size() > 0)
        {
            qDebug() << msg;

            if( msg == "@Sig=upgrade" )
            {
                emit sigUpgrade();
            }
        }
    }
}
