#ifndef VSSYSSETPAGE_H
#define VSSYSSETPAGE_H

#include <QWidget>
#include <QScrollArea>
#include "vsoptionpage.h"
#include "vsglobal.h"
#include <QWSServer>
#include <QMessageBox>

namespace Ui {
class VSSysSetPage;
}

class ConfigLanguagePage;
class ConfigDateTimePage;
class ConfigPoweroffPage;
class ConfigHardButtonPage;
class ConfigStandbyPage;
class KeyBoard;
class RestoreDialog;

class VSSysSetPage : public QWidget
{
    Q_OBJECT

public:
    explicit VSSysSetPage(QWidget *parent = 0);
    ~VSSysSetPage();

signals:
    void home();

    void goback();

protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event); //状态改变事件处理函数(语言改变事件处理在这里)

private slots:
    void on_button_home_clicked();
    void on_button_back_clicked();

signals:
    void restoreFactory();

public slots:
    void languageSet();
    void standbySet();
    void hardButtonSet();
    void poweroffSet();
    void nameSet();
    void nameSetConfirm(QString name);
    void serialSet();
    void serialSetConfirm(QString serial);
    void showMainWindow();
    void slotRestoreFactory();
    void slotRestoreAccept();
    void switchLanguage(LANGUAGE lan); //语言切换槽函数

private:
    void loadLanguage();
    void updateTranslator();
    void setScrollBarStyle();

private:
    Ui::VSSysSetPage *ui;

    QScrollArea         *m_scrollArea;
    VSOptionPage        *m_optionPage;

    ConfigLanguagePage  *m_configLanguagePage; //语言设置界面
    ConfigDateTimePage  *m_configDateTimePage; //时间日期设置界面
    ConfigPoweroffPage  *m_configPowerOffPage; //自动关机设置界面
    ConfigStandbyPage   *m_configStandbyPage; //自动待机设置界面
    ConfigHardButtonPage *m_configHardButtonPage; //硬按键设置界面
    KeyBoard    *m_keyBoard; //软键盘（设备名和设备序列号）

    RestoreDialog        *m_restoreDialog; //恢复厂家设置界面

    QPoint          m_pressPosition;

    LANGUAGE    m_curLanguage; //当前语言类型

    QTranslator *m_tr;

    bool        m_bMousePressed;

};

#endif // VSSYSSETPAGE_H
