#ifndef ITEMLABEL_H
#define ITEMLABEL_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class ItemLabel;
}

class ItemLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ItemLabel(const QPixmap pixmap, const QString file, QSize size, QWidget *parent = 0);
    ~ItemLabel();

    QPixmap  getPixmap() {return m_pixmap;}
    QString  getName() {return m_name;}
    void setSelected();
    void unSelected();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::ItemLabel *ui;

    QPixmap     m_pixmap;
    QString     m_name;

};

#endif // ITEMLABEL_H
