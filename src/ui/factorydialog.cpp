#include "factorydialog.h"
#include "ui_factorydialog.h"

RestoreDialog::RestoreDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RestoreDialog)
{
    ui->setupUi(this);
    ui->tipLabel->setAlignment(Qt::AlignHCenter);
}

RestoreDialog::~RestoreDialog()
{
    delete ui;
}

void RestoreDialog::mousePressEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void RestoreDialog::mouseReleaseEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void RestoreDialog::mouseMoveEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void RestoreDialog::mouseDoubleClickEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void RestoreDialog::on_button_reject_clicked()
{
    emit sigReject();
    hide();
}

void RestoreDialog::on_button_accept_clicked()
{
    emit sigAccept();
    hide();
}
