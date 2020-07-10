#include "configpagelanguage.h"
#include "ui_configpagelanguage.h"
#include "configmanager.h"

ConfigLanguagePage::ConfigLanguagePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigLanguagePage)
{
    ui->setupUi(this);
    ui->titleLabel->setText(QString(tr("Language Set")));
    ui->button_chinese->setCheckable(true);
    ui->button_english->setCheckable(true);

    loadConfig();
}

ConfigLanguagePage::~ConfigLanguagePage()
{
    delete ui;
}

void ConfigLanguagePage::on_pushButton_clicked()
{
    emit sigBack();
}

void ConfigLanguagePage::loadConfig()
{
    QVariant language;
    ConfigInstance->readSet(ITEM_LAN, language);
    //0:Chinese 1:English
    if(0 == language.toInt())
    {
        ui->button_chinese->setChecked(true);
        ui->button_english->setChecked(false);
    }
    else
    {
        ui->button_english->setChecked(true);
        ui->button_chinese->setChecked(false);
    }
}

void ConfigLanguagePage::on_button_chinese_clicked()
{
    if(ConfigInstance->writeSet(ITEM_LAN,0))
    {
        ui->button_chinese->setChecked(true);
        ui->button_english->setChecked(false);
        emit updateLanguage(LAN_CHINESE);
        emit sigBack();
    }
}

void ConfigLanguagePage::on_button_english_clicked()
{
    if(ConfigInstance->writeSet(ITEM_LAN,1))
    {
        ui->button_english->setChecked(true);
        ui->button_chinese->setChecked(false);
        emit updateLanguage(LAN_ENGLISH);
        emit sigBack();
    }
}

