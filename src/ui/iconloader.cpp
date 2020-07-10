#include "iconloader.h"
#include <QDebug>

IconLoader::IconLoader(QObject *parent)
    : QThread(parent)
{
    m_files.clear();
}

void IconLoader::run()
{
    if(m_files.isEmpty())
    {
        return;
    }

    foreach(QFileInfo file, m_files)
    {
        emit iconLoaded(file.fileName());
        msleep(470); //让图片逐张加载
    }
}
