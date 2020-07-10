#include "configmanager.h"
#include "vsassert.h"
#include "vsglobal.h"
#include <QMap>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>

//--------------------------------------------------------------
VSConfigManager *VSConfigManager::m_instance = NULL;
QMutex  VSConfigManager::m_mutex;

VSConfigManager::VSConfigManager()
{
    m_processer = new VSDataBase();
    m_instance = this;
    initSet();
}

VSConfigManager::~VSConfigManager()
{
    delete m_processer;
}

VSConfigManager *VSConfigManager::instance()
{
    if(NULL == m_instance)
    {
        m_mutex.lock();
        if(NULL == m_instance)
        {
            m_instance = new VSConfigManager();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

bool VSConfigManager::readSet(int key, QVariant &value)
{
    if(m_sets.contains(key))
    {
        value = m_sets.value(key);
        return true;
    }
    return false;
}

bool VSConfigManager::writeSet(int key, QVariant value)
{
    if(m_processer->writeData(key,value))
    {
        m_sets[key] = value;
        if(ITEM_AUTOPOWEROFF == key)
        {
            emit setPowerOffInterval();
        }
        if(ITEM_AUTOSTANDBY == key)
        {
            emit setStandbyInterval();
        }
        return true;
    }
    else
    {
        return false;
    }
}

void VSConfigManager::initSet()
{
    m_sets.clear();
    for(int key=ITEM_BACK_LIGHT; key<ITEM_MAX; key++)
    {
        QVariant value;
        if(m_processer->readData(key, value))
        {
            m_sets.insert(key, value);
        }
        else
        {
            //do something
        }
    }
}

void VSConfigManager::restoreFactory()
{
    m_processer->restoreData();
    initSet(); //不必考虑语言等不恢复的配置，因为所有的恢复都是在restoreData()中完成的
}

//------------------------------------------------------------
