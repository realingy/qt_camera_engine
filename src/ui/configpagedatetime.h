#ifndef CONFIGDATETIMEPAGE_H
#define CONFIGDATETIMEPAGE_H

#include <QWidget>

namespace Ui {
class ConfigDateTimePage;
}

class ConfigDateTimePage : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigDateTimePage(QWidget *parent = 0);
    ~ConfigDateTimePage();

signals:
    void sigBack();

private slots:
    void on_pushButton_clicked();

private:
    void loadConfig();

private:
    Ui::ConfigDateTimePage *ui;

};

#endif // CONFIGDATETIMEPAGE_H
