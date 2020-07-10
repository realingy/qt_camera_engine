#include "configpagedatetime.h"
#include "ui_configpagedatetime.h"
#include "configmanager.h"

ConfigDateTimePage::ConfigDateTimePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigDateTimePage)
{
    ui->setupUi(this);
    ui->titleLabel->setText(QString(tr("Language Set")));
    loadConfig();
}

ConfigDateTimePage::~ConfigDateTimePage()
{
    delete ui;
}

void ConfigDateTimePage::on_pushButton_clicked()
{
    emit sigBack();
}

void ConfigDateTimePage::loadConfig()
{
#if 0
    QVariant language;
    ConfigInstance->readSet(ITEM_LAN, language);
    //0: English 1:Chinese
    if(0 == language.toInt())
    {
        ui->button_english->setChecked(true);
        ui->button_chinese->setChecked(false);
    }
    else
    {
        ui->button_english->setChecked(false);
        ui->button_chinese->setChecked(true);
    }
#endif
}
