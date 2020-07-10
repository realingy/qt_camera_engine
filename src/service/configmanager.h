#ifndef VSCONFIGMANAGER_H
#define VSCONFIGMANAGER_H

#include <QWidget>
#include <QObject>
#include <QVariant>
#include <QMutex>
#include <QAtomicPointer>
#include "vsconfig.h"
#include "vsdatabase.h"

#define ConfigInstance VSConfigManager::instance()

/*!
 * \brief The VSConfigManager class
 * \brief 配置操作类，主要功能包括读配置/写配置/初始化数据库/管理QList
 */
class VSConfigManager : public QObject
{
    Q_OBJECT
public:
    VSConfigManager();
    ~VSConfigManager();

    static VSConfigManager* instance();

    //
    //get the config value of config key using const_iterator
    //
    //if get the config value of config key successfully,
    //set the config value to param value and return true.
    //else return false.
    //
    bool readSet(int key, QVariant &value);

    //set the config of config key(key) by value,
    //if the database seted successfully,return true.
    //else return false.
    //
    bool writeSet(int key, QVariant value);

    //initialize the QMap of all the config items
    void initSet();

    void restoreFactory();

signals:
    void setPowerOffInterval();

    void setStandbyInterval();

private:
    static VSConfigManager *m_instance;
    static QMutex  m_mutex;

    QMap<int , QVariant> m_sets;

    VSConfig  *m_processer;

};

#endif // VSCONFIGMANAGER_H
