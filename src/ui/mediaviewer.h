#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QImage>
#include <QDir>
#include <QListWidgetItem>
#include <QString>
#include <QStateMachine>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include "vsswipeview.h"
#include "deletedialog.h"

namespace Ui {
    class MediaViewer;
}

class MediaViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MediaViewer(QWidget *parent = 0);
    ~MediaViewer();

public:
    //第一次点击进入多媒体查看界面，需要设置当前文件
    void setCurrentFile(QFileInfo file, QFileInfoList fileList);

protected:
    void changeEvent(QEvent *event);//状态改变事件处理函数(语言改变事件处理在这里)
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void goback();
    void itemDeleted(QFileInfo );
    void firstImage();
    void lastImage();
    void oneImage();
    void resetSwitchEnable();
    void currentImageZoomIn();
    void currentImageNoneZoom();

public slots:
    void onImageClicked();
    void onImageDraged();
    void switchNext();
    void switchPrevious();

private slots:
    void on_gobackButton_clicked();
    void on_zoominButton_clicked();
    void on_zoomoutButton_clicked();
    void on_deleteButton_clicked();
    void deleteAccept();
    void deleteReject();
    void on_button_delete_clicked();
    void on_button_back_clicked();
    void on_button_display_clicked();
    void on_button_backward_clicked();
    void on_button_forward_clicked();
    void slotVideoDisplay();
    void on_videoSlider_valueChanged(int value);
    void enterHideState();

private:
    void updateImagePresent();
    void updateButtonStatus();
    void hideFunctionButtons();
    void showFunctionButtons();
    void updateMediaState();
    void videoAutoStop();
    void showDeleteDialog();

private:
    Ui::MediaViewer		*ui;

    VSSwipeView             *m_pSwipeView;
    DeleteDialog            *m_deleteDialog;

    QFileInfo                m_currentFile;
    QList<QFileInfo>         m_fileList;

    quint8 				 	 m_scale;
    quint8					 m_scaleStep;
    quint8					 m_minScale;
    quint8					 m_maxScale;

    bool    m_image;

    QTimer  *m_pAutoHideTimer;
    QTimer  *m_dispayTimer;

    int      m_displayValue;
    int      m_videoInterval;

    bool     m_videoPause;
    bool     m_active;

};

#endif // MEDIAVIEWER_H
