#ifndef CONFIGSTANDBYPAGE_H
#define CONFIGSTANDBYPAGE_H

#include <QWidget>
#include "configmanager.h"

namespace Ui {
class ConfigStandbyPage;
}

class ConfigStandbyPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigStandbyPage(QWidget *parent = 0);
    ~ConfigStandbyPage();
    void loadConfig();

signals:
    void sigBack();

private slots:
    void on_pushButton_clicked();
    void on_button_3_clicked();
    void on_button_5_clicked();
    void on_button_7_clicked();
    void on_button_10_clicked();
    void on_button_15_clicked();
    void on_button_close_clicked();

private:
    void clearChecked();

private:
    Ui::ConfigStandbyPage *ui;

};

#endif // CONFIGSTANDBYPAGE_H
