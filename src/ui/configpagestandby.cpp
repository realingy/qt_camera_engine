#include "configpagestandby.h"
#include "ui_configpagestandby.h"
#include "configmanager.h"

ConfigStandbyPage::ConfigStandbyPage(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::ConfigStandbyPage)
{
    ui->setupUi(this);
    ui->titleLabel->setText(QString(tr("Auto Standby Set")));

    ui->button_3->setCheckable(true);
    ui->button_5->setCheckable(true);
    ui->button_7->setCheckable(true);
    ui->button_10->setCheckable(true);
    ui->button_15->setCheckable(true);
    ui->button_close->setCheckable(true);

    loadConfig();
}

ConfigStandbyPage::~ConfigStandbyPage()
{
    delete ui;
}

void ConfigStandbyPage::on_pushButton_clicked()
{
    emit sigBack();
}

void ConfigStandbyPage::clearChecked()
{
    ui->button_3->setChecked(false);
    ui->button_5->setChecked(false);
    ui->button_7->setChecked(false);
    ui->button_10->setChecked(false);
    ui->button_15->setChecked(false);
    ui->button_close->setChecked(false);
}

void ConfigStandbyPage::loadConfig()
{
    QVariant interval;
    ConfigInstance->readSet(ITEM_AUTOSTANDBY, interval);

    //3: 3min 5: 5min
    //7: 7min 10: 10min
    //15: 15min 0: close
    switch (interval.toInt()) {
        case 3:
            ui->button_3->setChecked(true);
            break;
        case 5:
            ui->button_5->setChecked(true);
            break;
        case 7:
            ui->button_7->setChecked(true);
            break;
        case 10:
            ui->button_10->setChecked(true);
            break;
        case 15:
            ui->button_15->setChecked(true);
            break;
        case -1:
            ui->button_close->setChecked(true);
            break;
        default:
            break;
    }
}

void ConfigStandbyPage::on_button_3_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOSTANDBY, 3))
    {
        clearChecked();
        ui->button_3->setChecked(true);
        emit sigBack();
    }
}

void ConfigStandbyPage::on_button_5_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOSTANDBY, 5))
    {
        clearChecked();
        ui->button_5->setChecked(true);
        emit sigBack();
    }
}

void ConfigStandbyPage::on_button_7_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOSTANDBY, 7))
    {
        clearChecked();
        ui->button_7->setChecked(true);
        emit sigBack();
    }
}

void ConfigStandbyPage::on_button_10_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOSTANDBY, 10))
    {
        clearChecked();
        ui->button_10->setChecked(true);
        emit sigBack();
    }
}

void ConfigStandbyPage::on_button_15_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOSTANDBY, 15))
    {
        clearChecked();
        ui->button_15->setChecked(true);
        emit sigBack();
    }
}

void ConfigStandbyPage::on_button_close_clicked()
{
    if(ConfigInstance->writeSet(ITEM_AUTOSTANDBY, -1))
    {
        clearChecked();
        ui->button_close->setChecked(true);
        emit sigBack();
    }
}
