#include "mp4reader.h"
#include <QFileInfo>
#include <QDebug>

MP4Reader *MP4Reader::m_instance = NULL;
QMutex MP4Reader::m_mutex;

MP4Reader::MP4Reader()
  : m_pHandle(NULL)
  , m_videoTrackID(0)
  , m_videoFrameCounts(-1)
  , m_maxFrameSize(-1)
{
    m_instance = this;
}

MP4Reader::~MP4Reader()
{
#ifdef __arm__
    if(m_pHandle)
        MP4Close(m_pHandle);
#endif
}

MP4Reader *MP4Reader::instance()
{
    if(NULL == m_instance)
    {
        m_mutex.lock();
        if(NULL == m_instance)
        {
            m_instance = new MP4Reader;
        }
        m_mutex.unlock();
    }
    return m_instance;
}

int MP4Reader::open(QString fileName,QByteArray& sps,QByteArray& pps)
{
#ifdef __arm__
    if(m_pHandle)
    {
        MP4Close(m_pHandle);
    }

    {
        QFileInfo mp4file(fileName);
        if(!mp4file.exists())
            return -1;
    }

    m_pHandle = MP4Read(fileName.toLatin1().constData());

    if(!m_pHandle)
        return -1;

    try{
        if(getVideoTrack() <= 0)
            throw QString("Can not find video track");

        if(getMaxFrameCount() <= 0)
            throw QString("Invalid frame number");

        if(getMaxFrameSize() <= 0)
            throw QString("Invalid frame size");

        if(readH264Headers(sps,pps) < 0)
            throw QString("Can not read SPS & PPS");

        if(sps.size() == 0)
            throw QString("SPS is empty");
    }
    catch ( QString exception)
    {
        qDebug() << exception;
        MP4Close(m_pHandle);
        m_pHandle = NULL;
        return -1;
    }
#else
    Q_UNUSED(fileName)
    Q_UNUSED(sps)
    Q_UNUSED(pps)
#endif
    return 0;
}

bool MP4Reader::setFile(QString fileName, int &size)
{
#ifdef __arm__
    qDebug() << "1111111111111111: " << fileName;
    if(m_pHandle)
    {
        qDebug() << "22222222222222222";
        MP4Close(m_pHandle);
    }

    m_pHandle = MP4Read(fileName.toLatin1().constData());

    if(!m_pHandle)
    {
        qDebug() << "33333333333333333";
        return false;
    }

    if((m_videoTrackID=getVideoTrack()) <= 0)
    {
        qDebug() << "44444444444444444";
        return false;
    }

    size = MP4GetSampleSize(m_pHandle, m_videoTrackID, 1);
    qDebug() << "5555555555555555: " << size;

    return true;

#else
    Q_UNUSED(fileName)
#endif
}

int MP4Reader::readH264Headers(QByteArray &sps,QByteArray &pps)
{
#ifdef __arm__
    if(!m_pHandle)
        return -1;

    sps.clear();
    pps.clear();

    unsigned char **spsArray=NULL;
    uint32_t *spsSizeArray=NULL;
    unsigned char **ppsArray=NULL;
    uint32_t *ppsSizeArray=NULL;
    if(!MP4GetTrackH264SeqPictHeaders(m_pHandle,m_videoTrackID,
                                  &spsArray,&spsSizeArray,&ppsArray,&ppsSizeArray))
    {
        return -1;
    }

    QByteArray prefix;
    prefix.resize(4);
    prefix[0] = 0;
    prefix[1] = 0;
    prefix[2] = 0;
    prefix[3] = 1;

    if(spsArray)
    {
        for(int i = 0; spsArray[i] != NULL;i++)
        {
            sps.append(prefix);
            sps.append((const char *)spsArray[i],spsSizeArray[i]);
            free(spsArray[i]);
        }
        free(spsArray);
    }

    if(ppsArray)
    {
        for(int i = 0; ppsArray[i] != NULL;i++)
        {
            pps.append(prefix);
            pps.append((const char *)ppsArray[i],ppsSizeArray[i]);
            free(ppsArray[i]);
        }
        free(ppsArray);
    }
    return 0;
#else
    Q_UNUSED(sps)
    Q_UNUSED(pps)
    return 0;
#endif
}

int MP4Reader::getFrameSize(uint id)
{
#ifdef __arm__
    if(!m_pHandle)
        return -1;
    if(id == MP4_INVALID_SAMPLE_ID)
    {
        return -1;
    }
    else
    {
        if(id > getMaxFrameCount())
        {
            return -1;
        }
    }

    return MP4GetSampleSize(m_pHandle,m_videoTrackID,id);
#else
    Q_UNUSED(id)
    return 0;
#endif
}

int MP4Reader::readFrame(MP4SampleId sampleID, char *pBuf, uint32_t size)
{
#ifdef __arm__
    qDebug() << "!#$%#%^&^&^*&^";
    if(!m_pHandle)
    {
        qDebug() << "7777777777777777";
        return -1;
    }

    if(!pBuf)
    {
        qDebug() << "8888888888888888";
        return -1;
    }

    if(sampleID == MP4_INVALID_SAMPLE_ID)
    {
        qDebug() << "9999999999999999";
        return -1;
    }
    else
    {
        if(sampleID > getMaxFrameCount())
        {
            qDebug() << "101010101010101010";
            return -1;
        }
    }

    if(MP4ReadSample(m_pHandle,m_videoTrackID,sampleID,(uint8_t**)&pBuf,&size))
    {
        int i = 0;
        for(i = 0;i < 3;i++)
            pBuf[i]= 0;
        pBuf[i]=1;
        return size;
    }
    else
        return -1;
#else
    Q_UNUSED(sampleID)
    Q_UNUSED(pBuf)
    Q_UNUSED(size)
    return 0;
#endif
}

#if 0
int MP4Reader::readFrame(MP4SampleId sampleID, char *pBuf, uint32_t size)
{
#ifdef __arm__
    if(!m_pHandle)
        return -1;

    if(!pBuf)
        return -1;

    if(sampleID == MP4_INVALID_SAMPLE_ID)
    {
        return -1;
    }
#if 0
    else
    {
        if(sampleID > getMaxFrameCount())
            return -1;
    }
#endif

    if(MP4ReadSample(m_pHandle,m_videoTrackID,sampleID,(uint8_t**)&pBuf,&size))
    {
        int i = 0;
        for(i = 0;i < 3;i++)
            pBuf[i]= 0;
        pBuf[i]=1;
        return size;
    }
    else
        return -1;
#else
    Q_UNUSED(sampleID)
    Q_UNUSED(pBuf)
    Q_UNUSED(size)
    return 0;
#endif
}
#endif

int MP4Reader::getMaxFrameSize()
{
#ifdef __arm__
    if(!m_pHandle)
        return -1;
    if(m_maxFrameSize < 0)
        m_maxFrameSize = MP4GetTrackMaxSampleSize(m_pHandle,m_videoTrackID);
    return m_maxFrameSize;
#else
    return 0;
#endif
}

uint MP4Reader::getMaxFrameCount()
{
#ifdef __arm__
    if(!m_pHandle)
        return -1;
   if(m_videoFrameCounts< 0)
        m_videoFrameCounts= MP4GetTrackNumberOfSamples(m_pHandle,m_videoTrackID);
   return m_videoFrameCounts;
#else
    return 0;
#endif
}

int MP4Reader::getVideoTrack()
{
#ifdef __arm__
    if(!m_pHandle)
        return -1;
    int trackCount = MP4GetNumberOfTracks(m_pHandle);
    printf("trackCount = %d\n",trackCount);
    if(trackCount <= 0)
        return -1;
    int id = 1;
    for(id = 1; id <= trackCount;id++)
    {
        const char *trackType = MP4GetTrackType(m_pHandle,id);
        if(!trackType)
            continue;
        if(!strcmp(trackType,MP4_VIDEO_TRACK_TYPE))
        {
            break;
        }
    }

    if(id == 0 || id > trackCount)
    {
        return 0;
    }
    else
    {
        m_videoTrackID = id;
        printf("video track id : %d\n",m_videoTrackID);
        return id;
    }
#else
    return 0;
#endif
}
