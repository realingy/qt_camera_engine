#ifndef VSGLOBAL_H
#define VSGLOBAL_H

#include <QString>

#define ROOT_DIR   "e:/usrdata/"
#define CONFIGDIR  "config/"
#define CONFIG     "config.db"
#define SYSDATADIR "sysdata/"
#define SYSCONFIG  "sysconfig"

///////////////DEFAULT CONFIG VALUE////////////////

//1-10
#define VALUE_BACK_LIGHT_DEFAULT 5
#define VALUE_LED_LIGHT_DEFAULT 5

//0:chinese 1:english
#define VALUE_LAN_DEFAULT 0

//0:yy-MM-dd 1:MM-dd-yy 2:dd-MM-yy
#define DEFAULT_DATE_FORMAT 0

//0:24h 1:12h
#define DEFAULT_TIME_FORMAT 0

//5:5min 10:10min 15:15min 20:20min 30:min 0:close(disable)
#define DEFAULT_AUTO_POWEROFF 15

//3:3min 5:5min 7:7min 10:10min 15:min 0:close(disable)
#define DEFAULT_AUTO_STANDBY 3

//0:enable 1:disable
#define DEFAULT_HARDBUTTON_ENABLE 0

//0:long press 1:double press
#define DEFAULT_HARDBUTTON_STYLE   0

//0:enable 1:disable
#define DEFAULT_LOGO_ENABLE 0

//0:enable 1:disable
#define DEFAULT_WIFI_ENABLE 1

#define DEFAULT_DEVICE_NAME "vs31_0001"

#define DEFAULT_DEVICE_SERIAL "vs31_0001"
///////////////DEFAULT CONFIG VALUE////////////////

#define BCKLGT_MIN 1
#define BCKLGT_MAX 10
#define BCKLGT_STEP 1

#define LEDLGT_MIN 1
#define LEDLGT_MAX 10
#define LEDLGT_STEP 1


#ifdef __arm__
#define MEDIA_FOLDER "/work/media/"
#else
#define MEDIA_FOLDER "/work/multimedia/"
#endif

//image scale factor
#define INITSCALE 1
#define SCALESTEP 2
#define MINSCALE 10
#define MAXSCALE 50

#define DELETE(p) do{		\
                        delete p;	\
                        p = NULL;	\
                    } while(0)

typedef enum
{
    LAN_CHINESE = 0,
    LAN_ENGLISH = 1
} LANGUAGE;

#ifdef __arm__
#define w_desk 480
#define h_desk 320
#else
#define w_desk 960
#define h_desk 640
#endif


#endif // VSGLOBAL_H
