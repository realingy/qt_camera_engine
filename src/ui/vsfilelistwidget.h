#ifndef VSFILELISTWIDGET_H
#define VSFILELISTWIDGET_H

#include <QListWidget>
#include <QTimer>

class VSFileListWidget : public QListWidget
{
    Q_OBJECT
public:
    VSFileListWidget(QWidget *parent=NULL);

signals:
    void selectionModeChanged(QAbstractItemView::SelectionMode mode);

public slots:
    void switchMode();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent*event);

private:
    QTimer *m_pSwitchModeTimer;
};

#endif // VSFILELISTWIDGET_H
