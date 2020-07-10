#ifndef VSMAINSTACKEDWIDGET_H
#define VSMAINSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include "filebrowser.h"
#include "vsslidemenu.h"
#include "vsoptionpage.h"
#include "vssyssetpage.h"
#include "ipcmanager.h"
#include "factorydialog.h"
#include "keypresscheck.h"
#include "powercheck.h"
#include "poweroffdialog.h"
#include "calibration.h"
#include "datetimeconfig.h"
#include "powersupplyprocess.h"
#include <sys/ioctl.h>

#define MXCFB_SET_GBL_ALPHA     _IOW('F', 0x21, struct mxcfb_gbl_alpha)
struct mxcfb_gbl_alpha
{
    int enable;
    int alpha;
};

class QStateMachine;
class QMouseEvent;
class QShowEvent;
class QTimer;

namespace Ui {
    class VSMainWindow;
}

class VSMainWindow : public QStackedWidget
{
    Q_OBJECT

public:
    explicit VSMainWindow(QWidget *parent = 0);
    ~VSMainWindow();

protected:
    void changeEvent(QEvent *event); //状态改变事件处理函数(语言改变事件处理在这里)

signals:
    void showToolbar();
    void hideToolbar();
    void sigEnterRecordState();
    void sigExitRecordState();

public slots:
    void showHomePage();
    void showSlideMenu();
    void showFileBrowserPage();
    void updateCurrentTime();
    void setBackLight(int value);
    void setLedLight(int value);
    void showSysSetPage();
    void slotRestoreFactory();
    void handleNewPicture(QString fileName);
    void onFileDeleted();
    void showPowerDialog();
    void forcePowerOff();
    void rejectPoweroff();
    void slotPoweroff();
    void showSlider();
    void hideSlider();
    void takePhotoHardKey(bool);
    void recordHardKey(bool);
    void sliderValueChange(int change);
    void setWifiIcon(bool enable);
    void dateTimeConfig();
    void updatePowerState(EnumState state);
    void updatePowerCapacity(EnumCapacity capacity);

protected:
    bool event(QEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private slots:
    void exitActiveState();
    void enterActiveState();
    void on_ledSlider_valueChanged(int value);
    void slotFrameChanged(int);
    void updateMenuState();
    void enterRecordState();
    void exitRecordState();
    void on_button_photo_pressed();
    void on_button_photo_released();
    void updateRecIcon();
    void updateRecTime();

private:
    void loadConfig();
    void loadWifiConfig();
    int connectToServer();
    void updateBrowseIcon(QString fileName);
    //0:image 1:video
    QString generateNewName(int img);
    bool CtrlUILayerTransparent(bool enable);
    void calibratefinish();     //
    void startRecord();
    void stopRecord();
    void takeNewPicture();
    void loadPowerState();
    void loadPowerCapacity();

private:
    Ui::VSMainWindow 	*ui;

    QStateMachine *machine;
    QState *m_pActiveState;
    QState *m_pHideState;
    QState *m_pTakePhotoState;
    QState *m_pRecordingState;

    VSSlideMenu  *m_pSlideMenu;  //下拉菜单
    QPoint		  m_startPoint; //

    QTimer      *m_pAutoHideTimer; //状态机控制操作栏自动隐藏的计时器
    QTimer	    *m_pUpdateTimer; //时钟更新计时器
    QTimer	    *m_pFliker; //录像图标闪烁计时器
    QTimer	    *m_pRecordTimer; //录像计时器
    QTime        m_time;
    QString      m_strTime;

    FileBrowser 			*m_pFileBrowser;   //文件预览界面
    VSSysSetPage            *m_sysSetPage; //系统设置界面
    PageDatetime            *m_pageDatetime; //日期时间设置界面

    PowerSupplyProcess      *m_powerProc;  //电池处理器
    PoweroffDialog   *m_powerDialog; //关机界面

    KeyPressCheck    m_keyCheck; //硬操作按键检测线程
    PowerCheck       m_powerCheck; //关机按键监测线程

    QTimeLine 	 m_menuAnimator; //控制下拉菜单移动的动画
    qreal 		 m_distance;

    int         m_uiFbFd;
    bool	    m_isActiveState;
    bool	    m_slideMenuIsShow;
    bool        m_mousePress;
    bool        m_bRecordState;
    bool        m_bRecIcon;
    bool    m_mouseEnter;
};

#endif // VSMAINSTACKEDWIDGET_H
