#include "vsmainwindow.h"
#include "ui_vsmainwindow.h"
#include "vsdatabase.h"
#include "vsconfig.h"
#include "configmanager.h"
#include "vsglobal.h"
#include "lightcontroller.h"
#include "ipcmanager.h"
#include "vsmainwindow.h"
#include "ui_vsmainwindow.h"
#include "vsdatabase.h"
#include "vsconfig.h"
#include "configmanager.h"
#include "vsglobal.h"
#include "lightcontroller.h"
#include "ipcmanager.h"
#include <QDateTime>
#include <QDebug>
#include <QDirIterator>
#include <QDateTime>
#include <QDebug>
// #include <QDirIterator>
#include <QProcess>
#include <QMetaType>
#include <unistd.h>
#include <fcntl.h>

#define HEIGHT_TOPZONE 30 //顶部区域高度

VSMainWindow::VSMainWindow(QWidget *parent)
    : QStackedWidget(parent)
    , ui(new Ui::VSMainWindow)
    , m_pSlideMenu(NULL)
    , m_pFileBrowser(NULL)
    , m_sysSetPage(NULL)
    , m_powerDialog(NULL)
    , m_slideMenuIsShow(false)
    , m_mousePress(false)
    , m_mouseEnter(false)
    , m_bRecordState(false)
    , m_bRecIcon(false)
    , m_pageDatetime(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

#ifdef __arm__
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    if((m_uiFbFd = ::open("/dev/fb0", O_RDWR))<0)
    {
        qDebug()<<"Open /dev/fb0 failed";
    }

    qRegisterMetaType<EnumState>("EnumState"); //注册元类型
    qRegisterMetaType<EnumCapacity>("EnumCapacity"); //注册元类型

    ui->timelabel->hide(); //
    ui->label_rec->hide(); //
    ui->ledSlider->setEnabled(false); //
    ui->label_charge->hide(); //

    m_pAutoHideTimer=new QTimer;
    m_pAutoHideTimer->setInterval(5000);
    m_pAutoHideTimer->setSingleShot(true);

    machine = new QStateMachine;

    m_pActiveState= new QState(machine);
    machine->setInitialState(m_pActiveState);
    m_isActiveState = true;

    m_pHideState= new QState(machine);

    //情景1：隐藏计时器超时，自动进入隐藏状态
    QSignalTransition *hideTransition = m_pActiveState->addTransition(m_pAutoHideTimer,
                                                                      SIGNAL(timeout()), m_pHideState);
    hideTransition->addAnimation(new QPropertyAnimation(ui->bottombar, "geometry"));
    hideTransition->addAnimation(new QPropertyAnimation(ui->topbar, "geometry"));

    //情景2：点击LCD，手动进入隐藏状态
    QSignalTransition *manualHideTransition = m_pActiveState->addTransition(this
                                                                  ,SIGNAL(hideToolbar()), m_pHideState);
    manualHideTransition->addAnimation(new QPropertyAnimation(ui->bottombar, "geometry"));
    manualHideTransition->addAnimation(new QPropertyAnimation(ui->topbar, "geometry"));

    //情景3：点击LCD，手动进入活动状态
    QSignalTransition *showTransition= m_pHideState->addTransition(this
                                                                  ,SIGNAL(showToolbar()), m_pActiveState);
    showTransition->addAnimation(new QPropertyAnimation(ui->bottombar, "geometry"));
    showTransition->addAnimation(new QPropertyAnimation(ui->topbar, "geometry"));

    //进入隐藏状态时，停止时钟的更新
    connect(m_pActiveState,SIGNAL(entered()),this,SLOT(enterActiveState()));
    connect(m_pActiveState,SIGNAL(exited()),this,SLOT(exitActiveState()));

    //活动状态的两种子状态：录像和拍照状态
    m_pTakePhotoState = new QState(m_pActiveState);
    m_pRecordingState = new QState(m_pActiveState);
    connect(m_pRecordingState, SIGNAL(entered()), SLOT(enterRecordState()));
    connect(m_pRecordingState, SIGNAL(exited()), SLOT(exitRecordState()));

    m_pActiveState->setInitialState(m_pTakePhotoState);
    //情景1：点击录像按钮，进入录像状态
    QSignalTransition *gotoRecordingTransition= m_pTakePhotoState->addTransition(ui->button_video,
                                                                            SIGNAL(clicked()), m_pRecordingState);
    gotoRecordingTransition->addAnimation(new QPropertyAnimation(ui->button_photo,"geometry"));
    gotoRecordingTransition->addAnimation(new QPropertyAnimation(ui->button_video,"geometry"));

    //情景2：点击硬按键录像，进入录像状态
    QSignalTransition *gotoRecordingTransition2 = m_pTakePhotoState->addTransition(this,
                                                                            SIGNAL(sigEnterRecordState()), m_pRecordingState);
    gotoRecordingTransition2->addAnimation(new QPropertyAnimation(ui->button_photo,"geometry"));
    gotoRecordingTransition2->addAnimation(new QPropertyAnimation(ui->button_video,"geometry"));

    //情景3：点击录像按钮停止录像，进图拍照状态
    QSignalTransition *gotoTakePhotoTransition = m_pRecordingState->addTransition(ui->button_video,
                                                                                  SIGNAL(clicked()), m_pTakePhotoState);
    gotoTakePhotoTransition->addAnimation(new QPropertyAnimation(ui->button_photo,"geometry"));
    gotoTakePhotoTransition->addAnimation(new QPropertyAnimation(ui->button_video,"geometry"));

    //情景4：点击硬按键拍照，进入拍照模式
    QSignalTransition *gotoTakePhotoTransition2 = m_pRecordingState->addTransition(this,
                                                                            SIGNAL(sigExitRecordState()), m_pTakePhotoState);
    gotoTakePhotoTransition2->addAnimation(new QPropertyAnimation(ui->button_photo,"geometry"));
    gotoTakePhotoTransition2->addAnimation(new QPropertyAnimation(ui->button_video,"geometry"));

    //安装事件过滤器
    ui->button_photo->installEventFilter(this);
    ui->button_video->installEventFilter(this);

    machine->start();
    m_pAutoHideTimer->start();
    connect(this,SIGNAL(showToolbar()),m_pAutoHideTimer,SLOT(start()));

    //文件预览界面
    m_pFileBrowser = new FileBrowser;
    addWidget(m_pFileBrowser);
    connect(m_pFileBrowser, SIGNAL(home()), SLOT(showHomePage()));
    connect(m_pFileBrowser, SIGNAL(fileDeleted()), SLOT(onFileDeleted()));
    showHomePage();

    //获取最新文件，更新缩略图
    QString file = m_pFileBrowser->getLastFile();
    updateBrowseIcon(file);

    connect(ui->button_filebrowser,SIGNAL(clicked()),SLOT(showFileBrowserPage()));

    m_pSlideMenu = new VSSlideMenu(this);
    m_pSlideMenu->raise();
    loadConfig();
    connect(m_pSlideMenu, SIGNAL(sigBackLight(int)), SLOT(setBackLight(int)));
    connect(m_pSlideMenu, SIGNAL(sigLedLight(int)), SLOT(setLedLight(int)));
    connect(m_pSlideMenu, SIGNAL(sigOptionPage()), SLOT(showSysSetPage()));
    connect(m_pSlideMenu, SIGNAL(sigWifiEnable(bool)), SLOT(setWifiIcon(bool)));
    connect(m_pSlideMenu, SIGNAL(sigDateTimeConfig()), SLOT(dateTimeConfig()));
    loadWifiConfig();

    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm");
    ui->label_time->setText(str);

    m_pUpdateTimer = new QTimer;
    m_pUpdateTimer->setInterval(1000);
    m_pUpdateTimer->start();
    connect(m_pUpdateTimer,SIGNAL(timeout()), SLOT(updateCurrentTime()));
    connect(m_pHideState,SIGNAL(entered()),m_pUpdateTimer,SLOT(stop()));
    connect(m_pHideState,SIGNAL(exited()),m_pUpdateTimer,SLOT(start()));

#ifdef __arm__
    m_powerProc = new PowerSupplyProcess;
    connect(m_powerProc, SIGNAL(sigPowerState(EnumState)), SLOT(updatePowerState(EnumState)), Qt::QueuedConnection);
    connect(m_powerProc, SIGNAL(sigPowerCapacity(EnumCapacity)), SLOT(updatePowerCapacity(EnumCapacity)), Qt::QueuedConnection);
    loadPowerState();
    loadPowerCapacity();
#endif

    ui->ledSlider->setRange(LEDLGT_MIN,LEDLGT_MAX);
    ui->ledSlider->setPageStep(LEDLGT_STEP);
    ui->ledSlider->setParent(ui->hideFrame);

    ui->hideFrame->setStyleSheet(QString("border: 0px solid transparent;"));
    connect(ui->hideFrame, SIGNAL(showSlider()), SLOT(showSlider()));
    connect(ui->hideFrame, SIGNAL(hideSlider()), SLOT(hideSlider()));
    connect(ui->hideFrame, SIGNAL(sliderValueChange(int)), SLOT(sliderValueChange(int)));
    connect(ui->ledSlider, SIGNAL(showSlider()), SLOT(showSlider()));
    connect(ui->ledSlider, SIGNAL(hideSlider()), SLOT(hideSlider()));
    ui->ledIcon->hide();

    m_keyCheck.start();
    connect(&m_keyCheck, SIGNAL(sigShortKeyPress(bool)),SLOT(takePhotoHardKey(bool)), Qt::QueuedConnection);
    connect(&m_keyCheck, SIGNAL(sigLongKeyPress(bool)), SLOT(recordHardKey(bool)), Qt::QueuedConnection);

    connect(&m_powerCheck, SIGNAL(showPowerDialog()), SLOT(showPowerDialog()));
    connect(&m_powerCheck, SIGNAL(powerOffForce()), SLOT(forcePowerOff()));

    connect(&m_menuAnimator,SIGNAL(frameChanged(int)),SLOT(slotFrameChanged(int)));
    connect(&m_menuAnimator,SIGNAL(finished()), SLOT(updateMenuState()));
    m_menuAnimator.setDuration(400);
    m_menuAnimator.setCurveShape(QTimeLine::LinearCurve);

    m_pFliker= new QTimer;
    m_pFliker->setInterval(500);
    connect(m_pFliker, SIGNAL(timeout()), SLOT(updateRecIcon()));

    m_pRecordTimer = new QTimer;
    m_pRecordTimer->setInterval(1000);
    connect(m_pRecordTimer, SIGNAL(timeout()), SLOT(updateRecTime()));

    connectToServer(); //连接服务器
}

VSMainWindow::~VSMainWindow()
{
    ::close(m_uiFbFd);
    m_keyCheck.stop();
    DELETE(m_pUpdateTimer);
    DELETE(m_sysSetPage);
    DELETE(m_pFileBrowser);
    delete ui;
}

checkFileNum::checkFileNumPro(){

        QDirIterator dirIterator(USB_SOURCE_DIR);
        int numFiles = QDir(USB_SOURCE_DIR).count();
        QString fileNames;
        while(dirIterator.hasNext()){
            fileNames = dirIterator.next();
            fileNames = dirIterator.fileName();
            qDebug() << numFiles;
            qDebug() << fileNames;
        }
}

/*状态改变事件处理函数(语言改变事件处理在这里)*/
void VSMainWindow::changeEvent(QEvent*event)
{
    switch (event->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        QWidget::changeEvent(event);
        break;
    }
}

void VSMainWindow::loadConfig()
{
    QVariant bcklgt;
    if(ConfigInstance->readSet(ITEM_BACK_LIGHT, bcklgt))
    {
#ifdef __arm__
        LightInstance->setBackLight(bcklgt.toInt());
#endif
        m_pSlideMenu->setSlidebacklgt(bcklgt.toInt());
    }

    QVariant ledlgt;
    if(ConfigInstance->readSet(ITEM_LED_LIGHT, ledlgt))
    {
#ifdef __arm__
        LightInstance->setLedLight(ledlgt.toInt());
#endif
        if(ledlgt.toInt() != m_pSlideMenu->getLedLight())
        {
            m_pSlideMenu->setSlideledlgt(ledlgt.toInt());
        }
        if(ledlgt.toInt() != ui->ledSlider->value())
        {
            ui->ledSlider->setValue(ledlgt.toInt());
        }
    }
}

/*! \brief VSMainWindow::loadWifiConfig 读取WiFi配置 */
void VSMainWindow::loadWifiConfig()
{
    QVariant bWifi;
    if(ConfigInstance->readSet(ITEM_WIFI_ENABLE,bWifi))
    {
        if(0 == bWifi.toInt())
        {
//            ui->label_wifi->setStyleSheet(QString("border-image: url(:/res/icons/ic_wifi_2.png);"));
            ui->label_wifi->show();
        }
        else
        {
//            ui->label_wifi->setStyleSheet(QString("background-color: transparent;"));
            ui->label_wifi->hide();
        }
    }
}

//事件过滤
bool VSMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    Q_UNUSED(event)
    if(dynamic_cast<QMouseEvent*>(event))
    {
        emit showToolbar();
    }
    return false;
}

bool VSMainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        //点击屏幕，隐藏/活动状态互相切换，录像过程中禁止此操作
        if(false == m_isActiveState && false == m_bRecordState)
            emit showToolbar();
        else if(true == m_isActiveState && false == m_bRecordState)
            emit hideToolbar();
    }
    else if(event->type() == QEvent::Show || event->type() == QEvent::Resize)
    {
        m_pActiveState->assignProperty(ui->bottombar, "geometry", ui->bottombar->geometry());
        m_pActiveState->assignProperty(ui->topbar, "geometry", ui->topbar->geometry());

        m_pHideState->assignProperty(ui->bottombar, "geometry", QRect(ui->bottombar->geometry().left(),
                                                                     ui->bottombar->geometry().bottom(),
                                                                     ui->bottombar->geometry().width(),
                                                                     ui->bottombar->geometry().height()));

        m_pHideState->assignProperty(ui->topbar, "geometry", QRect(ui->topbar->geometry().left(),
                                                                  ui->topbar->geometry().top()-ui->topbar->geometry().height(),
                                                                  ui->topbar->geometry().width(),
                                                                  ui->topbar->geometry().height()));

        m_pTakePhotoState->assignProperty(ui->button_photo, "geometry", ui->button_photo->geometry());
        m_pTakePhotoState->assignProperty(ui->button_video, "geometry",ui->button_video->geometry());

        m_pRecordingState->assignProperty(ui->button_video, "geometry", ui->button_photo->geometry());
        m_pRecordingState->assignProperty(ui->button_photo, "geometry",ui->button_video->geometry());
    }
    return QWidget::event(event);
}

/*! \brief VSMainWindow::showHomePage 从其他界面回到主界面 */
void VSMainWindow::showHomePage()
{
    CtrlUILayerTransparent(true); //图层透明

    //释放系统设置的内存
    if(m_sysSetPage)
    {
        removeWidget(m_sysSetPage);
        delete m_sysSetPage;
        m_sysSetPage = NULL;
    }

    setCurrentWidget(ui->homepage);

    if(m_pSlideMenu)
    {
        m_pSlideMenu->raise();
    }

//    m_pAutoHideTimer->start(); //隐藏计时器恢复
}

//从其他界面回到下拉菜单界面
void VSMainWindow::showSlideMenu()
{
    CtrlUILayerTransparent(true);
    //释放其他界面内存
    if(m_sysSetPage)
    {
        removeWidget(m_sysSetPage);
        delete m_sysSetPage;
        m_sysSetPage = NULL;
    }
    if(m_pageDatetime)
    {
        removeWidget(m_pageDatetime);
        delete m_pageDatetime;
        m_pageDatetime = NULL;
    }

    setCurrentWidget(ui->homepage);

//    m_pAutoHideTimer->start();

    m_pSlideMenu->raise();
    m_pSlideMenu->move(0,0);
    m_slideMenuIsShow = true;
}

/*! \brief VSMainWindow::showFileBrowserPage 进入文件预览界面*/
void VSMainWindow::showFileBrowserPage()
{
    CtrlUILayerTransparent(false); //图层不透明

    setCurrentWidget(m_pFileBrowser);

//    m_pAutoHideTimer->stop();
    m_pFileBrowser->StartIconLoad(); //点击文件浏览按钮时再开始加载图片，以加快界面程序启动
}

void VSMainWindow::mousePressEvent(QMouseEvent *event)
{
    m_mousePress = true;
    m_startPoint = event->pos();

    QWidget::mousePressEvent(event);
}

void VSMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePress)
    {
        if(m_slideMenuIsShow == false && m_startPoint.y() >= HEIGHT_TOPZONE)
            return;

        if(m_slideMenuIsShow == true && m_startPoint.y() <= height()*3/4)
            return;

        m_distance = event->pos().y() - m_startPoint.y();
        if(m_slideMenuIsShow == false
                && m_startPoint.y() < HEIGHT_TOPZONE
                && m_startPoint.y() < event->pos().y()
                && event->pos().y() <= height())
        {
            m_pSlideMenu->move(0, m_distance - height());
        }
        else if(m_slideMenuIsShow == true
                && m_startPoint.y() > height()*3/4
                && m_startPoint.y() > event->pos().y()
                && event->pos().y() > 0)
        {
            m_pSlideMenu->move(0, m_distance);
        }
    }
    else if(m_mouseEnter)
    {
        m_distance = event->pos().y();
        if(m_slideMenuIsShow == false
            && event->pos().y() <= height())
        {
            m_pSlideMenu->move(0, m_distance - height());
        }
        else if(m_slideMenuIsShow == true
                && event->pos().y() > 0)
        {
            m_pSlideMenu->move(0, m_distance);
        }
    }

    QWidget::mouseMoveEvent(event);
}

void VSMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_mousePress)
    {
        m_mousePress = false;
        if(m_slideMenuIsShow == false && m_startPoint.y() >= HEIGHT_TOPZONE)
            return;

        if(m_slideMenuIsShow == true && m_startPoint.y() <= height()*3/4)
            return;

        if(event->pos().y() > this->height())
        {
            m_pSlideMenu->move(0,0);
        }

        qint32 start = 0, end = 0;
        if(m_slideMenuIsShow == false
                && m_startPoint.y() < HEIGHT_TOPZONE
                && event->pos().y() >= height()/2)
        {
            if((m_distance - height())*1000/height() < 0)
            {
                start = (m_distance - height())*1000/height();
                end = 0;
            }
        }
        else if(m_slideMenuIsShow == false
                && m_startPoint.y() < HEIGHT_TOPZONE
                && event->pos().y() < height()/2)
        {
            start = (m_distance - height())*1000/height();
            end = -1000;
        }
        else if(m_slideMenuIsShow == true
                && m_startPoint.y() > height()*3/4
                && event->pos().y() >= height()/2
                && event->pos().y() < m_startPoint.y())
        {
            start = m_distance*1000/height();
            end = 0;
        }
        else if(m_slideMenuIsShow == true
                && m_startPoint.y() > height()*3/4
                && event->pos().y() < height()/2)
        {
            start = m_distance*1000/height();
            end = -1000;
        }

        if(QTimeLine::NotRunning == m_menuAnimator.state())
        {
            m_menuAnimator.setFrameRange(start,end);
            m_menuAnimator.start();
        }
    }
    if(m_mouseEnter)
    {
        m_mouseEnter = false;

        if(event->pos().y() > this->height())
        {
            m_pSlideMenu->move(0,0);
        }

        qint32 start = 0, end = 0;
        if(m_slideMenuIsShow == false
                && event->pos().y() >= height()/2)
        {
            if((m_distance - height())*1000/height() < 0)
            {
                start = (m_distance - height())*1000/height();
                end = 0;
            }
        }
        else if(m_slideMenuIsShow == false
                && event->pos().y() < height()/2)
        {
            start = (m_distance - height())*1000/height();
            end = -1000;
        }
        else if(m_slideMenuIsShow == true
                && event->pos().y() >= height()/2)
        {
            start = m_distance*1000/height();
            end = 0;
        }
        else if(m_slideMenuIsShow == true
                && event->pos().y() < height()/2)
        {
            start = m_distance*1000/height();
            end = -1000;
        }

        if(QTimeLine::NotRunning == m_menuAnimator.state())
        {
            m_menuAnimator.setFrameRange(start,end);
            m_menuAnimator.start();
        }
    }

    QWidget::mouseReleaseEvent(event);
}

void VSMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_pSlideMenu->setGeometry(QRect(0, -height(), width(), height()));
    m_pFileBrowser->setGeometry(QRect(0, 0, width(), height()));
    //    m_pAutoHideTimer->start();
}

void VSMainWindow::enterEvent(QEvent*event)
{
    qDebug() << "mouse move in";
    m_mouseEnter = true;

    QWidget::enterEvent(event);
}

void VSMainWindow::leaveEvent(QEvent*event)
{
    qDebug() << "mouse move out";
    m_mouseEnter = false;

    QWidget::leaveEvent(event);
}

//更新系统时间
void VSMainWindow::updateCurrentTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm");
    if(str != ui->label_time->text())
    {
        ui->label_time->setText(str);
    }
}

void VSMainWindow::enterActiveState()
{
    m_isActiveState = true;
}

void VSMainWindow::exitActiveState()
{
    m_isActiveState = false;
}

void VSMainWindow::setBackLight(int value)
{
    if(ConfigInstance->writeSet(ITEM_BACK_LIGHT, (QVariant)value))
    {
#ifdef __arm__
        LightInstance->setBackLight(value);
#endif
    }
}

void VSMainWindow::setLedLight(int value)
{
    if(ConfigInstance->writeSet(ITEM_LED_LIGHT, (QVariant)value))
    {
#ifdef __arm__
        LightInstance->setLedLight(value);
#endif
        if(value != ui->ledSlider->value())
        {
            ui->ledSlider->setValue(value);
        }
    }
}

/*!
 * \brief VSMainWindow::showSysSetPage 进入系统设置界面
 */
void VSMainWindow::showSysSetPage()
{
    CtrlUILayerTransparent(false);
    if(m_slideMenuIsShow)
    {
        if(!m_sysSetPage)
        {
            m_sysSetPage = new VSSysSetPage();
            addWidget(m_sysSetPage);
            connect(m_sysSetPage, SIGNAL(home()), SLOT(showHomePage()));
            connect(m_sysSetPage, SIGNAL(goback()), SLOT(showSlideMenu()));
            connect(m_sysSetPage, SIGNAL(restoreFactory()), SLOT(slotRestoreFactory()));
        }
        setCurrentWidget(m_sysSetPage);
        m_pAutoHideTimer->stop();
        m_pSlideMenu->move(0, -height());
        m_slideMenuIsShow = false;
    }
}

void VSMainWindow::slotRestoreFactory()
{
    loadConfig();
}

int VSMainWindow::connectToServer()
{
    connect(IpcInstance, SIGNAL(newPicture(QString)), SLOT(handleNewPicture(QString)));
    if(IpcInstance->connectToServer() < 0)
    {
        qDebug()<<"connect to server failed";
        return -1;
    }
    else
    {
        qDebug()<<"connect to server success";
        return 0;
    }
}

void VSMainWindow::handleNewPicture(QString fileName)
{
    m_pFileBrowser->loadNewMedia(fileName);
    QString path = MEDIA_FOLDER;
    updateBrowseIcon(path+fileName);
}

void VSMainWindow::updateBrowseIcon(QString fileName)
{
    QPixmap p(fileName);
    quint32 width = p.width();
    quint32 height = p.height();
    QPixmap pix = p.copy(QRect((width-height)/2, 0, height, height));
    ui->button_filebrowser->setIconSize(QSize(ui->button_filebrowser->geometry().width()
                                              ,ui->button_filebrowser->geometry().height()));
    ui->button_filebrowser->setIcon(pix);
}

QString VSMainWindow::generateNewName(int img)
{
    QDateTime time = QDateTime::currentDateTime();

    QString name;
    if(0 == img)
    {
        name = time.toString("yyyyMMddhhmmss%1.jpg");
        qint64 sub = 0;
        QList<QString> names = m_pFileBrowser->getFileNames();

        while(names.contains(name.arg(sub)))
        {
            sub++;
        }

        return name.arg(sub);
    }
    else if(1 == img)
    {
        name = time.toString("yyyyMMddhhmmss");
        return name.append(QString(".mp4"));
    }
}

bool VSMainWindow::CtrlUILayerTransparent(bool enable)
{
    mxcfb_gbl_alpha alpha;
    alpha.alpha = 127;
    alpha.enable = enable ? 1 : 0;

    if(ioctl(m_uiFbFd, MXCFB_SET_GBL_ALPHA, &alpha)  < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void VSMainWindow::calibratefinish()
{
    m_pAutoHideTimer->start();
}

void VSMainWindow::onFileDeleted()
{
    QString file = m_pFileBrowser->getLastFile();
    updateBrowseIcon(file);
}

void VSMainWindow::showPowerDialog()
{
    if(m_powerDialog)
        delete m_powerDialog;

    m_powerDialog = new PoweroffDialog(this);
    m_pAutoHideTimer->stop();
    connect(m_powerDialog, SIGNAL(sigAccept()), SLOT(slotPoweroff()));
    connect(m_powerDialog, SIGNAL(sigReject()), SLOT(rejectPoweroff()));
    m_powerDialog->setGeometry(QRect(0,0,this->width(),this->height()));
    m_powerDialog->show();
}

void VSMainWindow::forcePowerOff()
{
    slotPoweroff();
}

void VSMainWindow::rejectPoweroff()
{
    m_pAutoHideTimer->start();
}

void VSMainWindow::slotPoweroff()
{
    qDebug("the system is going poweroff!");
    QProcess::execute("poweroff");
}

void VSMainWindow::on_ledSlider_valueChanged(int value)
{
    if(ConfigInstance->writeSet(ITEM_LED_LIGHT, (QVariant)value))
    {
#ifdef __arm__
        LightInstance->setLedLight(value);
#endif
        if(value != m_pSlideMenu->getLedLight())
        {
            m_pSlideMenu->setSlideledlgt(value);
        }
    }
}

void VSMainWindow::showSlider()
{
    if(false == m_bRecordState)
    {
        ui->topbar->hide();
        ui->bottombar->hide();
        m_pAutoHideTimer->stop();
    }

    ui->ledIcon->show();
    QString styleShow = QString(" \
                                QSlider{ \
                                    max-width: 70px; \
                                    min-width: 70px;   \
                                    background: rgba(165, 192, 120, 0); \
                                }\
                                QSlider::groove{    \
                                    border: 1px solid rgba(114,159,207,50);/*lightblue;*/  \
                                    width:  5px;  \
                                    border-radius: 3px; \
                                    background: rgba(238,238,2,50);  \
                                }  \
                                   \
                                QSlider::add-page {  \
                                    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1578ba, stop:1 #1578ba); \
                                    border: 1 px solid rgba(32, 92, 160,0); \
                                    border-radius: 3px; \
                                } \
                                  \
                                QSlider::sub-page{  \
                                    background-color: #8c8c8c;\
                                    border: 1 px solid rgba(32, 92,160,0); \
                                    border-radius: 3px; \
                                } \
                                  \
                                QSlider::handle{ \
                                    width: 60px; \
                                    height: 60px; \
                                    border-image: url(:/res/icons/ic_slider_handle.png); \
                                    margin: -22px -28px -20px -28px; \
                                }"
                                );
    ui->ledSlider->setStyleSheet(styleShow);
}

void VSMainWindow::hideSlider()
{
    if(false == m_bRecordState)
    {
        ui->topbar->show();
        ui->bottombar->show();
        m_pAutoHideTimer->start();
    }

    ui->ledIcon->hide();
    QString styleHide = QString(" \
                                QSlider{ \
                                    max-width: 70px; \
                                    min-width: 70px;   \
                                    background: rgba(165, 192, 120, 0); \
                                }\
                                QSlider::groove{    \
                                    border: 1px solid rgba(114,159,207,0);/*lightblue;*/  \
                                    width: 10px;  \
                                    border-radius: 4px; \
                                    background: rgba(238,238,2,0);  \
                                }  \
                                   \
                                QSlider::add-page {  \
                                    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(32, 92, 160,0), stop:1 rgba(32,92,160,0)); \
                                    border: 1 px solid rgba(32, 92, 160,0); \
                                    width: 5px; \
                                } \
                                  \
                                QSlider::sub-page{  \
                                    background-color: rgba(123,12,143,0);  \
                                    border: 1 px solid rgba(32, 92,160,0); \
                                    width: 5px; \
                                } \
                                  \
                                QSlider::handle{ \
                                    width: 60px; \
                                    height: 60px; \
                                    /*border-image: url(:/res/icons/slider_handle_hide.png);*/ \
                                    border-color: rgba(255,255,255,0); \
                                    margin: -18px -22px -18px -22px; \
                                }");
    ui->ledSlider->setEnabled(false);
    ui->ledSlider->setStyleSheet(styleHide);
}

void VSMainWindow::takePhotoHardKey(bool start)
{
    qDebug() << "take a photo by hard key!";
    if(true == start)
    {
        ui->button_photo->setStyleSheet(QString("border-image: url(:/res/icons/ic_photo_checked.png);"));
    }
    else
    {
        ui->button_photo->setStyleSheet(QString("border-image: url(:/res/icons/ic_photo_unchecked.png);"));
        takeNewPicture();
    }
}

void VSMainWindow::recordHardKey(bool record)
{
    qDebug() << "record by hard key!";
    if(true == record)
    {
        startRecord();
    }
    else
    {
        stopRecord();
    }
}

void VSMainWindow::sliderValueChange(int change)
{
    showSlider();
    int value = ui->ledSlider->value();
    value += change;

    if(value < 1)
        value = 1;
    else if(value > 10)
        value = 10;

    ui->ledSlider->setValue(value);
}

void VSMainWindow::setWifiIcon(bool enable)
{
    if(true == enable)
    {
        ui->label_wifi->setStyleSheet(QString("border-image: url(:/res/icons/ic_wifi_2.png);"));
    }
    else
    {
        ui->label_wifi->setStyleSheet(QString("background-color: transparent;"));
    }
}

void VSMainWindow::dateTimeConfig()
{
    CtrlUILayerTransparent(false);
    if(m_slideMenuIsShow)
    {
        m_pSlideMenu->move(0, -height());
        m_slideMenuIsShow = false;
        if(!m_pageDatetime)
        {
            m_pageDatetime = new PageDatetime();
            connect(m_pageDatetime, SIGNAL(goback()), SLOT(showSlideMenu()));
        }
        addWidget(m_pageDatetime);
        setCurrentWidget(m_pageDatetime);
        m_pAutoHideTimer->stop();
    }
}

void VSMainWindow::loadPowerState()
{
    QProcess proc;
    QStringList cmd;

    //bash -c "cmd" : bash cmd
    cmd << "-c";
    cmd << "cat /sys/class/power_supply/max8903-charger/uevent | grep STATUS";

    proc.start("/bin/bash", cmd);
    proc.waitForReadyRead();
    proc.waitForFinished();

    char *msg = proc.readAll().data();
    char *state;
    if(!strncmp(msg, "POWER_SUPPLY_STATUS=", 20))
    {
        msg += 20;
        state = msg;
    }

    if(!strncmp(state, "Discharging", 11) || !strncmp(state, "Not charging", 12))
    {
        updatePowerState(State_Discharging);
        m_powerProc->setPowerState(State_Discharging);
    }
    else if(!strncmp(state, "Charging", 8))
    {
        updatePowerState(State_Charging);
        m_powerProc->setPowerState(State_Charging);
    }
}

void VSMainWindow::loadPowerCapacity()
{
    QProcess proc;
    QStringList cmd;

    cmd << "-c";
    cmd << "cat /sys/class/power_supply/max8903-charger/uevent | grep CAPACITY=";

    proc.start("/bin/bash", cmd);
    proc.waitForReadyRead();
    proc.waitForFinished();

    char *msg = proc.readAll().data();
    char *cap;
    if(!strncmp(msg, "POWER_SUPPLY_CAPACITY=", 22))
    {
        msg += 22;
        cap = msg;
    }

    int capacity = QString(cap).toInt();
    if(capacity < 10)
    {
        updatePowerCapacity(Capacity_0);
        m_powerProc->setPowerCapacity(Capacity_0);
    }
    else if(capacity >= 10 && capacity < 20)
    {
        updatePowerCapacity(Capacity_20_Red);
        m_powerProc->setPowerCapacity(Capacity_20_Red);
    }
    else if(capacity >= 20 && capacity < 40)
    {
        updatePowerCapacity(Capacity_20);
        m_powerProc->setPowerCapacity(Capacity_20);
    }
    else if(capacity >= 40 && capacity < 60)
    {
        updatePowerCapacity(Capacity_40);
        m_powerProc->setPowerCapacity(Capacity_40);
    }
    else if(capacity >= 60 && capacity < 80)
    {
        updatePowerCapacity(Capacity_60);
        m_powerProc->setPowerCapacity(Capacity_60);
    }
    else if(capacity >= 80 && capacity < 95)
    {
        updatePowerCapacity(Capacity_80);
        m_powerProc->setPowerCapacity(Capacity_80);
    }
    else if(capacity >= 95 && capacity < 100)
    {
        updatePowerCapacity(Capacity_100);
        m_powerProc->setPowerCapacity(Capacity_100);
    }
}

void VSMainWindow::updatePowerState(EnumState state)
{
    qDebug() << "update power state!";
    if(state == State_Discharging)
    {
        ui->label_charge->hide();
    }
    else if(state == State_Charging)
    {
        ui->label_charge->show();
    }
}

void VSMainWindow::updatePowerCapacity(EnumCapacity capacity)
{
    qDebug() << "update power capacity!";
    switch(capacity)
    {
        case Capacity_0:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_0.png)");
            break;
        case Capacity_20_Red:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_20_red.png)");
            break;
        case Capacity_20:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_20.png)");
            break;
        case Capacity_40:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_40.png)");
            break;
        case Capacity_60:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_60.png)");
            break;
        case Capacity_80:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_80.png)");
            break;
        case Capacity_100:
            ui->label_battery->setProperty("styleSheet", "border-image:url(://res/icons/ic_battery_100.png)");
            break;
    }
}

void VSMainWindow::slotFrameChanged(int pos)
{
    qreal offset= height()*pos/1000;
    m_pSlideMenu->move(0,offset);
}

void VSMainWindow::updateMenuState()
{
    if(-height() ==  m_pSlideMenu->geometry().y())
    {
        m_slideMenuIsShow = false;
    }
    else
    {
        m_slideMenuIsShow = true;
    }
}

void VSMainWindow::enterRecordState()
{
    startRecord();
}


void VSMainWindow::exitRecordState()
{
    stopRecord();
}

void VSMainWindow::startRecord()
{
    emit sigEnterRecordState();
    ui->button_video->setStyleSheet(QString("border-image: url(:/res/icons/ic_record_checked.png);"));
    ui->button_filebrowser->hide();
    ui->label_rec->show();
    ui->timelabel->show();
    ui->timelabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->topbar->hide();
    m_bRecordState = true;
    m_pAutoHideTimer->stop();
    m_pFliker->start();
    m_pRecordTimer->start();
    m_time = QTime::currentTime();
    ui->timelabel->setText(QString("00:00"));

//    if(connectToServer() < 0)
//        return;
    QString fileName = generateNewName(1);
    QString folder = MEDIA_FOLDER;
    IpcInstance->startRecordVideo(folder, fileName);
}

void VSMainWindow::stopRecord()
{
    emit sigExitRecordState();
    ui->button_video->setStyleSheet(QString("border-image: url(:/res/icons/ic_record_unchecked.png);"));
    ui->button_filebrowser->show();
    ui->label_rec->hide();
    ui->timelabel->hide();
    ui->topbar->show();
    m_bRecordState = false;
    m_pAutoHideTimer->start();
    m_pFliker->stop();
    m_pRecordTimer->stop();

//    if(connectToServer() < 0)
//        return;
    IpcInstance->stopRecordVideo();
}

void VSMainWindow::on_button_photo_pressed()
{
    ui->button_photo->setStyleSheet(QString("border-image: url(:/res/icons/ic_photo_checked.png);"));
}

void VSMainWindow::on_button_photo_released()
{
    ui->button_photo->setStyleSheet(QString("border-image: url(:/res/icons/ic_photo_unchecked.png);"));
    takeNewPicture();
}

void VSMainWindow::takeNewPicture()
{
    if(connectToServer() < 0)
        return;
    QString fileName = generateNewName(0);
    QString folder = MEDIA_FOLDER;
    IpcInstance->takeAPicture(folder, fileName);
}

void VSMainWindow::updateRecIcon()
{
    m_bRecIcon = !m_bRecIcon;
    if(m_bRecIcon)
    {
        ui->label_rec->hide();
    }
    else
    {
        ui->label_rec->show();
    }
}

void VSMainWindow::updateRecTime()
{
    QTime currTime = QTime::currentTime();
    int t = m_time.msecsTo(currTime);
    QTime showTime(0,0,0,0);
    showTime = showTime.addMSecs(t);
    m_strTime = showTime.toString("mm:ss");
    ui->timelabel->setText(m_strTime);
}
