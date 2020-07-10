#include "configpagehardbutton.h"
#include "ui_configpagehardbutton.h"
#include "configmanager.h"

ConfigHardButtonPage::ConfigHardButtonPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigHardButtonPage)
{
    ui->setupUi(this);
    ui->titleLabel->setText(QString(tr("HardButton Set")));
    ui->button_longpress->setCheckable(true);
    ui->button_doublepress->setCheckable(true);
    loadConfig();
}

ConfigHardButtonPage::~ConfigHardButtonPage()
{
    delete ui;
}

void ConfigHardButtonPage::on_pushButton_clicked()
{
    emit sigBack();
}

void ConfigHardButtonPage::loadConfig()
{
    QVariant buttonstyle;
    ConfigInstance->readSet(ITEM_HARDBUTTON_STYLE, buttonstyle);
    //0:long press 1:double press
    if(0 == buttonstyle.toInt())
    {
        ui->button_longpress->setChecked(true);
        ui->button_doublepress->setChecked(false);
    }
    else
    {
        ui->button_doublepress->setChecked(true);
        ui->button_longpress->setChecked(false);
    }
}

void ConfigHardButtonPage::on_button_longpress_clicked()
{
    if(ConfigInstance->writeSet(ITEM_HARDBUTTON_STYLE,0))
    {
        ui->button_longpress->setChecked(true);
        ui->button_doublepress->setChecked(false);
        emit sigBack();
    }
}

void ConfigHardButtonPage::on_button_doublepress_clicked()
{
    if(ConfigInstance->writeSet(ITEM_HARDBUTTON_STYLE,1))
    {
        ui->button_doublepress->setChecked(true);
        ui->button_longpress->setChecked(false);
        emit sigBack();
    }
}
