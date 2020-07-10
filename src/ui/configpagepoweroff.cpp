#include "configpagepoweroff.h"
#include "ui_configpagepoweroff.h"
#include "configmanager.h"

ConfigPoweroffPage::ConfigPoweroffPage(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::ConfigPoweroffPage)
{
    ui->setupUi(this);
    ui->titleLabel->setText(QString(tr("Auto Poweroff Set")));

    ui->button_5->setCheckable(true);
    ui->button_10->setCheckable(true);
    ui->button_15->setCheckable(true);
    ui->button_20->setCheckable(true);
    ui->button_30->setCheckable(true);
    ui->button_close->setCheckable(true);

    loadConfig();
}

ConfigPoweroffPage::~ConfigPoweroffPage()
{
    delete ui;
}

void ConfigPoweroffPage::on_pushButton_clicked()
{
    emit sigBack();
}

void ConfigPoweroffPage::loadConfig()
{
    QVariant interval;
    ConfigInstance->readSet(ITEM_AUTOPOWEROFF, interval);

    //5: 5min 10: 10min
    //15: 15min 20: 20min
    //30: 30min 0: close
    switch (interval.toInt()) {
        case 5:
            ui->button_5->setChecked(true);
            break;
        case 10:
            ui->button_10->setChecked(true);
            break;
        case 15:
            ui->button_15->setChecked(true);
            break;
        case 20:
            ui->button_20->setChecked(true);
            break;
        case 30:
            ui->button_30->setChecked(true);
            break;
        case -1:
            ui->button_close->setChecked(true);
            break;
        default:
            break;
    }
}

void ConfigPoweroffPage::on_button_5_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOPOWEROFF, 5))
    {
        clearChecked();
        ui->button_5->setChecked(true);
        emit sigBack();
    }
}

void ConfigPoweroffPage::on_button_10_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOPOWEROFF, 10))
    {
        clearChecked();
        ui->button_10->setChecked(true);
        emit sigBack();
    }
}

void ConfigPoweroffPage::on_button_15_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOPOWEROFF, 15))
    {
        clearChecked();
        ui->button_15->setChecked(true);
        emit sigBack();
    }
}

void ConfigPoweroffPage::on_button_20_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOPOWEROFF, 20))
    {
        clearChecked();
        ui->button_20->setChecked(true);
        emit sigBack();
    }
}

void ConfigPoweroffPage::on_button_30_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOPOWEROFF, 30))
    {
        clearChecked();
        ui->button_30->setChecked(true);
        emit sigBack();
    }
}

void ConfigPoweroffPage::on_button_close_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOPOWEROFF, -1))
    {
        clearChecked();
        ui->button_close->setChecked(true);
        emit sigBack();
    }
}

void ConfigPoweroffPage::clearChecked()
{
    ui->button_5->setChecked(false);
    ui->button_10->setChecked(false);
    ui->button_15->setChecked(false);
    ui->button_20->setChecked(false);
    ui->button_30->setChecked(false);
    ui->button_close->setChecked(false);
}
