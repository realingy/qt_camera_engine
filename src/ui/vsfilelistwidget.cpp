#include "vsfilelistwidget.h"

VSFileListWidget::VSFileListWidget(QWidget *parent)
    :QListWidget(parent)
{
    m_pSwitchModeTimer=new QTimer;
    m_pSwitchModeTimer->setInterval(600);
    m_pSwitchModeTimer->setSingleShot(true);
    connect(m_pSwitchModeTimer, SIGNAL(timeout()), SLOT(switchMode()));
}

void VSFileListWidget::switchMode()
{
    if(this->selectionMode() == QAbstractItemView::SingleSelection)
    {
        setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else
    {
        setSelectionMode(QAbstractItemView::SingleSelection);
    }
    emit selectionModeChanged(selectionMode());
}

void VSFileListWidget::mousePressEvent(QMouseEvent* event)
{
    if(this->count() && this->selectionMode() == QAbstractItemView::SingleSelection)
        m_pSwitchModeTimer->start();
    QListWidget::mousePressEvent(event);
    QWidget::mousePressEvent(event);

}

void VSFileListWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(m_pSwitchModeTimer->isActive())
        m_pSwitchModeTimer->stop();
    QListWidget::mouseReleaseEvent(event);
    QWidget::mouseReleaseEvent(event);
}

void VSFileListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pSwitchModeTimer->isActive())
        m_pSwitchModeTimer->stop();
    QListWidget::mouseMoveEvent(event);
    QWidget::mouseMoveEvent(event);
}
