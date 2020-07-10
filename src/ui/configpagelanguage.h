#ifndef CONFIGLANGUAGEPAGE_H
#define CONFIGLANGUAGEPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QTranslator>
#include "vsglobal.h"

namespace Ui {
class ConfigLanguagePage;
}

class ConfigLanguagePage : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigLanguagePage(QWidget *parent = 0);
    ~ConfigLanguagePage();

signals:
    void sigBack();
    void updateLanguage(LANGUAGE lan);

private slots:
    void on_pushButton_clicked();
    void on_button_chinese_clicked();
    void on_button_english_clicked();

private:
    void loadConfig();

private:
    Ui::ConfigLanguagePage *ui;

};

#endif // CONFIGLANGUAGEPAGE_H
