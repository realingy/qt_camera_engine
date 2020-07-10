#ifndef VSDIALOG_H
#define VSDIALOG_H

#include <QWidget>

namespace Ui {
class RestoreDialog;
}

class RestoreDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RestoreDialog(QWidget *parent = 0);
    ~RestoreDialog();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void sigAccept();
    void sigReject();

private slots:
    void on_button_reject_clicked();
    void on_button_accept_clicked();

private:
    Ui::RestoreDialog *ui;
};

#endif // VSDIALOG_H
