#ifndef VSDATABASE_H
#define VSDATABASE_H

#include <QSqlDatabase>
#include <QVariant>
#include <QWidget>
#include <QMutex>
#include "vsconfig.h"

class VSDataBase : public VSConfig
{
public:
    VSDataBase();
    ~VSDataBase();

    virtual void restoreData();
    virtual void initData();

    //
    //get the query of key
    //if get successfully,
    //set the query to value and return true.
    //else return false.
    virtual bool readData(int key, QVariant &value);

    //
    //write the query of database
    //if the database write successfully,return true.
    //else return false.
    virtual bool writeData(int key, QVariant value);

private:
    bool createTable();
    void updateTable();
    bool isItemExist(const int &key) const;
    bool insertTableItem(const int key, const QVariant value);

private:
    static QMutex m_mutex;

    QSqlDatabase m_db;

    static const QString TABLE_NAME_CONFIG;

    QMap<int, QVariant> m_defaultitems;

};

#endif // VSDATABASE_H
