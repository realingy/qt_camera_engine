#ifndef VSPROTOCOL_H
#define VSPROTOCOL_H

#include <QByteArray>
#include <QList>

#define HEADER_1 '#'
#define HEADER_2 '$'
#define HEADER_1_OFFSET 0
#define HEADER_2_OFFSET 1
#define LENGTH_H_OFFSET 2
#define LENGTH_L_OFFSET 3
#define CMD_ID_OFFSET 4
#define SUBCMD_NUM_OFFSET 5
#define MIN_PACKET_SIZE 8

class VSProtocol
{
public:
    static QByteArray builder(qint8 ID, QList<QByteArray> data);
private:
    static quint16 caculateChecksum(const char *pData, quint32 size);
};

#endif // VSPROTOCOL_H
