#ifndef MSGPROC_H
#define MSGPROC_H

#include <QThread>
#include <QTimer>

class MsgProc : public QThread
{
    Q_OBJECT
public:
    MsgProc();
    ~MsgProc();

public slots:
    void    slotUpgradeAccept();

    void    slotUpgradeReject();

protected:
    void run();

signals:
    void sigUpgrade();

private:
    QString m_fifo;

};

#endif // MSGPROC_H
