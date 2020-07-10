#ifndef MP4READER_H
#define MP4READER_H

#include <QByteArray>
#include <QString>
#include "mp4v2/mp4v2.h"
#include "mp4v2/file.h"
#include <QDebug>
#include <QMutex>

#define VideoReader MP4Reader::instance()

class MP4Reader
{
public:
    explicit MP4Reader();

    ~MP4Reader();

    static MP4Reader *instance();

    int open(QString fileName, QByteArray& sps, QByteArray& pps);

    bool setFile(QString fileName, int &size);

    int readFrame(MP4SampleId sampleID, char *pBuf, uint32_t size);

    int getMaxFrameSize();

    uint getMaxFrameCount(); //huoqu zhen shu

    int getFrameSize(uint id);

    MP4Timestamp getFrameTime(MP4SampleId id)
    {
        return MP4GetSampleTime(m_pHandle,m_videoTrackID,id);
    }

    uint32_t getTimeScale()
    {
        return MP4GetTrackTimeScale(m_pHandle,m_videoTrackID);
    }

    MP4Duration getDuration()
    {
        MP4Duration trackDuration = MP4GetTrackDuration(m_pHandle,m_videoTrackID);
        return MP4ConvertFromTrackDuration(m_pHandle,m_videoTrackID,trackDuration,MP4_MSECS_TIME_SCALE);
    }

private:
    int getVideoTrack();

    int readH264Headers(QByteArray& sps, QByteArray& pps);

private:
    MP4FileHandle m_pHandle;

    static MP4Reader *m_instance;
    static QMutex  m_mutex;

    int m_videoTrackID;
    int m_videoFrameCounts;
    int m_maxFrameSize;

};


#endif // MP4READER_H
