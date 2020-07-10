#include "vsprotocol.h"

QByteArray VSProtocol::builder(qint8 ID, QList<QByteArray> data)
{
    QByteArray packet;
    packet.clear();
    quint16 headSize = 4;
    packet[HEADER_1_OFFSET]=HEADER_1;
    packet[HEADER_2_OFFSET]=HEADER_2;
    packet[CMD_ID_OFFSET]=ID;
    packet[SUBCMD_NUM_OFFSET]=data.size();
    foreach (QByteArray b, data)
    {
        packet.append(b);
    }
    packet[LENGTH_H_OFFSET]=((packet.size()-headSize) >> 8) & 0xFF;
    packet[LENGTH_L_OFFSET]=(packet.size()-headSize) & 0xFF;

    const char *pData=packet.constData();
    quint16 checksum=caculateChecksum(pData+headSize,packet.size()-headSize);
    packet[packet.size()]=(checksum >> 8) & 0xFF;
    packet[packet.size()]=checksum & 0xFF;

    return packet;
}

quint16 VSProtocol::caculateChecksum(const char *pData, quint32 size)
{
    return qChecksum(pData,size);
}
