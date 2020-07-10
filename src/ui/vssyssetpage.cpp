#include "vssyssetpage.h"
#include "ui_vssyssetpage.h"
#include "vsglobal.h"
#include "configpagedatetime.h"
#include "configmanager.h"
#include "configpagepoweroff.h"
#include "configpagestandby.h"
#include "configpagehardbutton.h"
#include "configpagelanguage.h"
#include "keyboard.h"
#include "factorydialog.h"
#include "configmanager.h"
#include "vsconfig.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QTimer>

VSSysSetPage::VSSysSetPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VSSysSetPage)
    , m_bMousePressed(false)
    , m_configLanguagePage(NULL)
    , m_configDateTimePage(NULL)
    , m_configPowerOffPage(NULL)
    , m_configStandbyPage(NULL)
    , m_configHardButtonPage(NULL)
    , m_keyBoard(NULL)
    , m_restoreDialog(NULL)
{
    ui->setupUi(this);

    m_scrollArea = new QScrollArea(ui->centralwidget);
    m_optionPage = new VSOptionPage();

    connect(m_optionPage, SIGNAL(restoreFactory()), SLOT(slotRestoreFactory()));
    connect(m_optionPage, SIGNAL(sigLanguageSet()), SLOT(languageSet()));
    connect(m_optionPage, SIGNAL(sigStandbySet()), SLOT(standbySet()));
    connect(m_optionPage, SIGNAL(sigHardButtonSet()), SLOT(hardButtonSet()));
    connect(m_optionPage, SIGNAL(sigPowerOffSet()), SLOT(poweroffSet()));
    connect(m_optionPage, SIGNAL(sigNameSet()), SLOT(nameSet()));
    connect(m_optionPage, SIGNAL(sigSerialSet()), SLOT(serialSet()));

    m_scrollArea->setWidget(m_optionPage);   //将OptionPage作为滑动界面
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScrollBarStyle();

    qRegisterMetaType<LANGUAGE>("LANGUAGE");

    m_tr = new QTranslator(this);
    if(!m_tr->load("En_Ch"))   //加载翻译文件
    {
        qDebug("Load failed");
    }

    loadLanguage();
}

VSSysSetPage::~VSSysSetPage()
{
    DELETE(m_restoreDialog);
    delete ui;
}

void VSSysSetPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_scrollArea->setFixedSize(ui->widget->width(), ui->widget->height());
    m_optionPage->setFixedSize(ui->widget->width(), ui->widget->height()*3);
}

/*状态改变事件处理函数(语言改变事件处理在这里)*/
void VSSysSetPage::changeEvent(QEvent*event)
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

void VSSysSetPage::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_bMousePressed)
    {
        return;
    }

    QPoint currentPt = e->pos();

    int dist = m_pressPosition.y() - currentPt.y();

    m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() + dist);

    m_pressPosition = currentPt;
}

void VSSysSetPage::mousePressEvent(QMouseEvent *e)
{
    m_bMousePressed = true;
    m_pressPosition = e->pos();
}

void VSSysSetPage::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    m_bMousePressed = false;

    m_pressPosition.setX(0);
    m_pressPosition.setY(0);
}

void VSSysSetPage::on_button_home_clicked()
{
    emit home();
}

void VSSysSetPage::on_button_back_clicked()
{
    emit goback();
}

void VSSysSetPage::setScrollBarStyle()
{
    m_scrollArea->setStyleSheet(QString("\
               QScrollBar::vertical {\
                  background-color: transparent;\
                  border-color: transparent;\
                  width: 10px;\
                  /*margin:150px 0px 150px 0px;*/\
               }\
               QScrollBar::handle:vertical {\
                  border-image: url(:/res/icons/ic_scrollbar_handle.png);\
                  min-height: 60px;\
                  width: 10px;\
               }\
               QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {\
                  subcontrol-origin: margin;\
                  height: 30px;\
                  background-color: transparent;\
               }\
               QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                  background-color: transparent;\
                  border:0px solid transparent;\
               }\
               QScrollArea {\
                  border:0px solid transparent;\
               }\
               "));
}

//设置语言槽函数
void VSSysSetPage::languageSet()
{
    if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
    {
        if(!m_configLanguagePage)
        {
            m_configLanguagePage = new ConfigLanguagePage;
        }
        connect(m_configLanguagePage, SIGNAL(sigBack()), SLOT(showMainWindow()));
        connect(m_configLanguagePage, SIGNAL(updateLanguage(LANGUAGE)), SLOT(switchLanguage(LANGUAGE)));

        pParent->addWidget(m_configLanguagePage);
        pParent->setCurrentWidget(m_configLanguagePage);
    }
}

//设置自动待机槽函数
void VSSysSetPage::standbySet()
{
    if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
    {
        if(!m_configStandbyPage)
        {
            m_configStandbyPage = new ConfigStandbyPage;
        }
        connect(m_configStandbyPage, SIGNAL(sigBack()), SLOT(showMainWindow()));

        pParent->addWidget(m_configStandbyPage);
        pParent->setCurrentWidget(m_configStandbyPage);
    }
}

//设置硬按键行为槽函数
void VSSysSetPage::hardButtonSet()
{
    if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
    {
        if(!m_configHardButtonPage)
        {
            m_configHardButtonPage = new ConfigHardButtonPage;
        }
        connect(m_configHardButtonPage, SIGNAL(sigBack()), SLOT(showMainWindow()));

        pParent->addWidget(m_configHardButtonPage);
        pParent->setCurrentWidget(m_configHardButtonPage);
    }
}

//自动关机槽函数
void VSSysSetPage::poweroffSet()
{
    if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
    {
        if(!m_configPowerOffPage)
        {
            m_configPowerOffPage = new ConfigPoweroffPage;
        }
        connect(m_configPowerOffPage, SIGNAL(sigBack()), SLOT(showMainWindow()));

        pParent->addWidget(m_configPowerOffPage);
        pParent->setCurrentWidget(m_configPowerOffPage);
    }
}

//设备名设置槽函数
void VSSysSetPage::nameSet()
{
    //soft keyboard
    if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
    {
        if(!m_keyBoard)
        {
            QVariant name;
            if(ConfigInstance->readSet(ITEM_DEVICE_NAME,name))
            {
                m_keyBoard = new KeyBoard(name.toString());
            }
        }
        connect(m_keyBoard, SIGNAL(sigBack()), SLOT(showMainWindow()));
        connect(m_keyBoard, SIGNAL(sigConfirmInput(QString)), SLOT(nameSetConfirm(QString)));
        pParent->addWidget(m_keyBoard);
        pParent->setCurrentWidget(m_keyBoard);
    }
}

//设备名设置确认保存槽函数
void VSSysSetPage::nameSetConfirm(QString name)
{
    if(ConfigInstance->writeSet(ITEM_DEVICE_NAME, name))
    {
        m_optionPage->readNameConfig(); //配置页面重新读取设备名配置
    }
}

//设备序列号设置槽函数
void VSSysSetPage::serialSet()
{
    //soft keyboard
    if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
    {
        if(!m_keyBoard)
        {
            QVariant serial;
            if(ConfigInstance->readSet(ITEM_DEVICE_SERIAL,serial))
            {
                m_keyBoard = new KeyBoard(serial.toString());
            }
        }
        connect(m_keyBoard, SIGNAL(sigConfirmInput(QString)), SLOT(serialSetConfirm(QString)));
        connect(m_keyBoard, SIGNAL(sigBack()), SLOT(showMainWindow()));
        pParent->addWidget(m_keyBoard);
        pParent->setCurrentWidget(m_keyBoard); //配置页面重新读取设备序列号配置
    }
}

//设备序列号设置确认保存槽函数
void VSSysSetPage::serialSetConfirm(QString serial)
{
    if(ConfigInstance->writeSet(ITEM_DEVICE_SERIAL, serial))
    {
        m_optionPage->readSerialConfig();
    }
}

/*!
 * \brief VSSysSetPage::showMainWindow 设置成功返回槽函数，需要配置界面重新读取配置更新显示
 */
void VSSysSetPage::showMainWindow()
{
    if(QStackedWidget* pParent = dynamic_cast<QStackedWidget *>(this->parent()))
    {
        if(m_configLanguagePage)
        {
            m_optionPage->readLanConfig();
            pParent->removeWidget(m_configLanguagePage);
            DELETE(m_configLanguagePage);
        }
        else if(m_configDateTimePage)
        {
            pParent->removeWidget(m_configDateTimePage);
            DELETE(m_configDateTimePage);
        }
        else if(m_configPowerOffPage)
        {
            m_optionPage->readAutoPoweroffConfig();
            pParent->removeWidget(m_configPowerOffPage);
            DELETE(m_configPowerOffPage);
        }
        else if(m_configStandbyPage)
        {
            m_optionPage->readAutoStandbyConfig();
            pParent->removeWidget(m_configStandbyPage);
            DELETE(m_configStandbyPage);
        }
        else if(m_configHardButtonPage)
        {
            m_optionPage->readHardbuttonConfig();
            pParent->removeWidget(m_configHardButtonPage);
            DELETE(m_configHardButtonPage);
        }
        else if(m_keyBoard)
        {
            pParent->removeWidget(m_keyBoard);
            DELETE(m_keyBoard);
        }
        pParent->setCurrentWidget(this);
    }
}

//恢复厂家设置槽函数
void VSSysSetPage::slotRestoreFactory()
{
    if(m_restoreDialog)
        delete m_restoreDialog;

    m_restoreDialog = new RestoreDialog(this);
    connect(m_restoreDialog, SIGNAL(sigAccept()), SLOT(slotRestoreAccept()));
    m_restoreDialog->setGeometry(QRect(0,0,this->width(),this->height()));
    m_restoreDialog->show();
}

//恢复厂家设置确认槽函数，需要配置界面重新读取配置并更新显示
void VSSysSetPage::slotRestoreAccept()
{
    ConfigInstance->restoreFactory();
    m_optionPage->loadConfig();
    emit  restoreFactory();
}

void VSSysSetPage::loadLanguage()
{
    QVariant language;
    ConfigInstance->readSet(ITEM_LAN, language);
    if(0 == language.toInt())
    {
        m_curLanguage = LAN_CHINESE;
    }
    else
    {
        m_curLanguage = LAN_ENGLISH;
    }
    updateTranslator();
}

void VSSysSetPage::updateTranslator()
{
    /*如果是中文*/
    if(m_curLanguage == LAN_CHINESE)
    {
        qApp->installTranslator(m_tr);
    }
    else
    {
        qApp->removeTranslator(m_tr);
    }
}

void VSSysSetPage::switchLanguage(LANGUAGE lan)
{
    LANGUAGE language;
    switch(lan)
    {
    case LAN_CHINESE://中文
        language = LAN_CHINESE;
        break;

    case LAN_ENGLISH://英文
        language = LAN_ENGLISH;
        break;

    default://其他
        language = LAN_CHINESE;
        break;
    }

    if(m_curLanguage != language)
    {
        if(language == LAN_ENGLISH)
            qApp->removeTranslator(m_tr);
        else
            qApp->installTranslator(m_tr);
    }
    m_curLanguage = language;
}
