#ifndef POWEROFFDIALOG_H
#define POWEROFFDIALOG_H

#include <QWidget>

namespace Ui {
class PoweroffDialog;
}

class PoweroffDialog : public QWidget
{
    Q_OBJECT

public:
    explicit PoweroffDialog(QWidget *parent = 0);
    ~PoweroffDialog();

signals:
    void sigAccept();
    void sigReject();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void on_button_accept_clicked();
    void on_button_reject_clicked();

private:
    Ui::PoweroffDialog *ui;
};

#endif // POWEROFFDIALOG_H
