#ifndef CONFIGHARDBUTTONPAGE_H
#define CONFIGHARDBUTTONPAGE_H

#include <QWidget>

namespace Ui {
class ConfigHardButtonPage;
}

class ConfigHardButtonPage: public QWidget
{
    Q_OBJECT

public:
    explicit ConfigHardButtonPage(QWidget *parent = 0);
    ~ConfigHardButtonPage();

signals:
    void sigBack();

private slots:
    void on_pushButton_clicked();
    void on_button_longpress_clicked();
    void on_button_doublepress_clicked();

private:
    void loadConfig();

private:
    Ui::ConfigHardButtonPage *ui;

};

#endif // CONFIGHARDBUTTONPAGE_H
