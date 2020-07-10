#ifndef CONFIGPOWEROFFPAGE_H
#define CONFIGPOWEROFFPAGE_H

#include <QWidget>
#include "configmanager.h"

namespace Ui {
class ConfigPoweroffPage;
}

class ConfigPoweroffPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigPoweroffPage(QWidget *parent = 0);
    ~ConfigPoweroffPage();
    void loadConfig();

signals:
    void sigBack();

private slots:
    void on_pushButton_clicked();
    void on_button_5_clicked();
    void on_button_10_clicked();
    void on_button_15_clicked();
    void on_button_20_clicked();
    void on_button_30_clicked();
    void on_button_close_clicked();

private:
    void clearChecked();

private:
    Ui::ConfigPoweroffPage *ui;

};

#endif // CONFIGPOWEROFFPAGE_H
