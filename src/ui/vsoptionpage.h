#ifndef VSOPTIONPAGE_H
#define VSOPTIONPAGE_H

#include <QWidget>
#include <QTranslator>
#include <QMetaType>
#include <QList>
#include <QFrame>
#include "triaxialprocess.h"

#define LISTLEN 100

namespace Ui {
class VSOptionPage;
}

class IpcManager;
class TriaxialProcess;

/*!
 * \brief The ItemFrame class
 * 子类化QFrame，用于整条设置项的响应
 */
class ItemFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ItemFrame(QWidget *parent = 0);
    ~ItemFrame();

signals:
    void itemClicked();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    bool m_scroll;
    bool m_press;

};

class VSOptionPage : public QWidget
{
    Q_OBJECT

public:
    explicit VSOptionPage(QWidget *parent = 0);
    ~VSOptionPage();

    void loadConfig();
    void readLanConfig();
    void readAutoPoweroffConfig();
    void readAutoStandbyConfig();
    void readHardbuttonConfig();
    void readLogoConfig();
    void readTimeFormatConfig();
    void readNameConfig();
    void readSerialConfig();

protected:
    void changeEvent(QEvent *event); //状态改变事件处理函数(语言改变事件处理在这里)

protected:

public slots:
    void setServerVersion(QString version);
    void updateTriaxial(EnumDirection direction, qlonglong value);

signals:
    void restoreFactory();
    void sigLanguageSet();
    void sigNameSet();
    void sigStandbySet();
    void sigPowerOffSet();
    void sigHardButtonSet();
    void sigSerialSet();
    void sigMotionless();

private slots:
    void on_button_restore_clicked();
    void on_button_lan_clicked();
    void on_button_name_clicked();
    void on_button_standby_clicked();
    void on_button_power_clicked();
    void on_button_hard_clicked();
    void on_switch_hardbutton_clicked();
    void on_switch_logo_clicked();
    void on_button_serial_clicked();
    void on_button_24h_clicked();
    void initConfig();

private:
    void deviceMotionless();

private:
    Ui::VSOptionPage *ui;

    QList<qlonglong>     m_listX;
    QList<qlonglong>     m_listY;
    QList<qlonglong>     m_listZ;

    bool m_bHardbutton;         //hard button enable

    bool m_bLogo;       //logo enable

    bool m_b24H;        //24 hour format enabel

    TriaxialProcess *m_triProcess;

};

#endif // VSOPTIONPAGE_H
