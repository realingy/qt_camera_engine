#include "poweroffdialog.h"
#include "ui_poweroffdialog.h"

PoweroffDialog::PoweroffDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PoweroffDialog)
{
    ui->setupUi(this);
}

PoweroffDialog::~PoweroffDialog()
{
    delete ui;
}

void PoweroffDialog::mousePressEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void PoweroffDialog::mouseMoveEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void PoweroffDialog::mouseReleaseEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void PoweroffDialog::mouseDoubleClickEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void PoweroffDialog::on_button_accept_clicked()
{
    emit sigAccept();
    hide();
}

void PoweroffDialog::on_button_reject_clicked()
{
    emit sigReject();
    hide();
}
