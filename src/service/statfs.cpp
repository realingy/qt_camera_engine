#include "statfs.h"

#ifndef WIN32
VSStatfs *VSStatfs::m_instance = NULL;
QMutex VSStatfs::m_mutex;

VSStatfs::VSStatfs()
{
    m_instance = this;
}

VSStatfs *VSStatfs::instance()
{
    if(NULL == m_instance)
    {
        m_mutex.lock();
        if(NULL == m_instance)
        {
            m_instance = new VSStatfs();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

//获取磁盘空间信息
int VSStatfs::getDiskInfo(const char *path, struct statfs *diskInfo)
{
    if(-1 == statfs(path, diskInfo))
    {
        return -1;
    }
    return 0;
}

//计算磁盘空间，计算结果以字符串的形式存储，单位为MB
int VSStatfs::calDiskInfo(qlonglong &diskTotal,qlonglong &diskAvail)
{
    struct statfs disk;
    memset(&disk, 0, sizeof(struct statfs));
    if(!getDiskInfo(DEFAULT_DISK_PATH, &disk))
    {
        struct statfs *diskInfo = &disk;
        qint64 blockSize=diskInfo->f_bsize; //每块包含字节大小

        diskTotal = diskInfo->f_blocks * blockSize;//磁盘总空间
        diskAvail = diskInfo->f_bavail * blockSize;//非超级用户可用空间

        return 0;
    }
    return -1;
}
#endif
