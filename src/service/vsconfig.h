#ifndef VSCONFIG_H
#define VSCONFIG_H

#include <QVariant>

enum ConfigItem
{
    ITEM_BACK_LIGHT = 0,
    ITEM_LED_LIGHT,
    ITEM_LAN,
    ITEM_DATE_FORMAT,
    ITEM_TIME_FORMAT,
    ITEM_AUTOPOWEROFF,
    ITEM_AUTOSTANDBY,
    ITEM_HARDBUTTON_ENABLE,
    ITEM_HARDBUTTON_STYLE,
    ITEM_LOGO_ENABLE,
    ITEM_WIFI_ENABLE,
    ITEM_DEVICE_NAME,
    ITEM_DEVICE_SERIAL,
    ITEM_MAX, //item sum
};

class VSConfig
{
public:
    VSConfig();
    virtual ~VSConfig();

    virtual void initData()=0;

    //
    //get the config data of database
    //if get the config value of config item successfully,
    //set the config data to param value and return true.
    //else return false.
    virtual bool readData(int key, QVariant &value)=0;

    //
    //write the config data of database
    //if the database write successfully,return true.
    //else return false.
    virtual bool writeData(int key, QVariant value)=0;

    virtual void restoreData()=0;

};

#endif // VSCONFIG_H
