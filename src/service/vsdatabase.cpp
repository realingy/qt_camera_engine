#include "vsdatabase.h"
#include "configmanager.h"
#include "vsglobal.h"
#include "vsassert.h"
#include <QDir>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

const QString VSDataBase::TABLE_NAME_CONFIG("Config");
QMutex VSDataBase::m_mutex;

VSDataBase::VSDataBase() : VSConfig()
{
    //init the default item list
    m_defaultitems.insert(ITEM_BACK_LIGHT,VALUE_BACK_LIGHT_DEFAULT);
    m_defaultitems.insert(ITEM_LED_LIGHT,VALUE_LED_LIGHT_DEFAULT);
    m_defaultitems.insert(ITEM_LAN,VALUE_LAN_DEFAULT);
    m_defaultitems.insert(ITEM_DATE_FORMAT,DEFAULT_DATE_FORMAT);
    m_defaultitems.insert(ITEM_TIME_FORMAT,DEFAULT_TIME_FORMAT);
    m_defaultitems.insert(ITEM_AUTOPOWEROFF,DEFAULT_AUTO_POWEROFF);
    m_defaultitems.insert(ITEM_AUTOSTANDBY,DEFAULT_AUTO_STANDBY);
    m_defaultitems.insert(ITEM_HARDBUTTON_ENABLE,DEFAULT_HARDBUTTON_ENABLE);
    m_defaultitems.insert(ITEM_HARDBUTTON_STYLE,DEFAULT_HARDBUTTON_STYLE);
    m_defaultitems.insert(ITEM_LOGO_ENABLE,DEFAULT_LOGO_ENABLE);
    m_defaultitems.insert(ITEM_WIFI_ENABLE,DEFAULT_WIFI_ENABLE);
    m_defaultitems.insert(ITEM_DEVICE_NAME,DEFAULT_DEVICE_NAME);
    m_defaultitems.insert(ITEM_DEVICE_SERIAL,DEFAULT_DEVICE_SERIAL);

    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName("config");
    }

    qDebug() << m_db.connectionName();
    qDebug() << m_db.databaseName();

    if (!m_db.open())
    {
        qDebug() << "Error: Failed to connect database." << m_db.lastError();
    }
    else
    {
        initData();
    }
}

VSDataBase::~VSDataBase()
{
    m_db.removeDatabase(m_db.connectionName());
    m_db.close();
}

void VSDataBase::initData()
{
    createTable();
    updateTable();
}

bool VSDataBase::readData(int key, QVariant &value)
{
    qDebug() << "read data. key: " << key;

    m_mutex.lock();
    QSqlQuery query(m_db);
    QString sql = QString("select * from %1 where key = %2").arg(TABLE_NAME_CONFIG).arg(key);
    if(query.exec(sql))
    {
        while(query.next())
        {
            value = query.value(1);  //select by the column number
//            value = query.value("value"); //select by the column lable
            m_mutex.unlock();
            return true;
        }
    }
    m_mutex.unlock();
    return false;
}

bool VSDataBase::writeData(int key, QVariant value)
{
    qDebug() << "write data. key: " << key;

    QSqlQuery query(m_db);
    QString sql_update = QString("update %1 set value = ? where key = ?").arg(TABLE_NAME_CONFIG);
    query.prepare(sql_update);
    query.addBindValue(value);
    query.addBindValue(key);

    if(!query.exec())
    {
        qDebug() << "write data fail." << query.lastError();

        return false;
    }

    query.finish();

    return true;
}

void VSDataBase::restoreData()
{
    qDebug() << "restore factory data";

    QMap<int, QVariant>::const_iterator it = m_defaultitems.constBegin();

    while(m_defaultitems.constEnd() != it)
    {
        //language will not been restored
        if(ITEM_LAN == it.key() || ITEM_WIFI_ENABLE == it.key() || ITEM_DEVICE_NAME  == it.key()
           || ITEM_LOGO_ENABLE == it.key())
        {
            ++it;
            continue;
        }
        writeData(it.key(), it.value());
        ++it;
    }
}

bool VSDataBase::createTable()
{
    QSqlQuery query(m_db);
    QString sql = QString("CREATE TABLE IF NOT EXISTS %1 ( key INTEGER PRIMARY KEY, value BLOB);").arg(TABLE_NAME_CONFIG);
    query.prepare(sql);
    if(!query.exec())
    {
        qDebug() << "creat table fail." << m_db.lastError();
        return false;
    }
    return true;
}

void VSDataBase::updateTable()
{
    QMap<int, QVariant>::const_iterator it = m_defaultitems.constBegin();

    while(m_defaultitems.constEnd() != it)
    {
        if(!isItemExist(it.key()))
        {
            insertTableItem(it.key(),it.value());
        }
        ++it;
    }
}

bool VSDataBase::isItemExist(const int &key) const
{
    QSqlQuery query = QSqlQuery(m_db);

//    QString sql = QString("select count(*) from sqlite_master where type='table', name='%1'").arg(tableName);
    QString sql = QString("select key from %1").arg(TABLE_NAME_CONFIG);

    if(query.exec(sql))
    {
        while(query.next())
        {
            if(query.value(0).toInt() == key)
            {
                return true;
            }
        }
    }
    return false;
}

bool VSDataBase::insertTableItem(const int key, const QVariant value)
{
    QSqlQuery query(m_db);
    QString insert_sql = QString("insert into %1 values (?, ?)").arg(TABLE_NAME_CONFIG);
    query.prepare(insert_sql);

    query.addBindValue(key);
    query.addBindValue(value);

    if(!query.exec())
    {
        return false;
    }

    query.finish();

    return true;
}
