#ifndef VSSLIDERMENU_H
#define VSSLIDERMENU_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class VSSlideMenu;
}

class VSSlideMenu : public QWidget
{
    Q_OBJECT
public:
    explicit VSSlideMenu(QWidget *parent = 0);
    ~VSSlideMenu();

    void setSlidebacklgt(int value);
    void setSlideledlgt(int value);
    int getLedLight();

signals:
    void sigBackLight(int value);
    void sigLedLight(int value);
    void sigOptionPage();
    void sigWifiEnable(bool enable);
    void sigDateTimeConfig();

protected:
    void changeEvent(QEvent *event);//状态改变事件处理函数(语言改变事件处理在这里)
    void mousePressEvent(QMouseEvent *event);

public slots:
    void updateCurrentTime();

private slots:
    void on_button_option_clicked();
    void clearClickCount();
    void on_sliderbcklgt_valueChanged(int value);
    void on_sliderledlgt_valueChanged(int value);
    void on_button_switch_clicked();

private:
    void setSwitchIcon();
    void loadWifiConfig();

private:
    Ui::VSSlideMenu	   *ui;

    QTimer  *m_timer;
    QTimer  *m_pUpdateTimer;

    quint8   m_count;

    bool     m_bWifi;

};

#endif // VSSLIDERMENU_H
