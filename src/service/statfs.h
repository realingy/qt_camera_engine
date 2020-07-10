#ifndef VSSTATFS_H
#define VSSTATFS_H

#include <QMutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <QString>

#ifndef WIN32
#include <sys/statfs.h>
#include <sys/vfs.h>

#ifdef __arm__
#define DEFAULT_DISK_PATH "/work"
#else
#define DEFAULT_DISK_PATH "/work"
#endif

#define Statfs VSStatfs::instance()

class VSStatfs
{
public:
    VSStatfs();

    static VSStatfs *instance();

    int calDiskInfo(qlonglong &, qlonglong &);

private:
    static VSStatfs *m_instance;

    static QMutex  m_mutex;

    int getDiskInfo(const char *path, struct statfs *diskInfo);

};
#endif

#endif // VSSTATFS_H
