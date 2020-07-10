#include "deletedialog.h"
#include "ui_deletedialog.h"
#include <QMouseEvent>

DeleteDialog::DeleteDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeleteDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowFlags(Qt::FramelessWindowHint);
}

DeleteDialog::~DeleteDialog()
{
    delete ui;
}

void DeleteDialog::mousePressEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void DeleteDialog::mouseMoveEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void DeleteDialog::mouseReleaseEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void DeleteDialog::mouseDoubleClickEvent(QMouseEvent*event)
{
    Q_UNUSED(event)
}

void DeleteDialog::on_button_accept_clicked()
{
    emit sigAccept();
}

void DeleteDialog::on_button_reject_clicked()
{
    emit sigReject();
}
