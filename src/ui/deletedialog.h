#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <QWidget>
#include <QEvent>

namespace Ui {
class DeleteDialog;
}

class DeleteDialog : public QWidget
{
    Q_OBJECT

public:
    explicit DeleteDialog(QWidget *parent = 0);
    ~DeleteDialog();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void sigAccept();
    void sigReject();

private slots:
    void on_button_accept_clicked();
    void on_button_reject_clicked();

private:
    Ui::DeleteDialog *ui;
};

#endif // DELETEDIALOG_H
