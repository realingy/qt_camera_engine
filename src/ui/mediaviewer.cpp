#include "mediaviewer.h"
#include "ui_mediaviewer.h"
#include <QMessageBox>
#include <QDebug>
#include <QDesktopWidget>
#include <QTimer>
#include "vsglobal.h"
#include "ipcmanager.h"

/*!
 * \brief MediaViewer::MediaViewer
 * 文件查看类，包括查看图片和视频
 * \author 5005965
 * \date  2018.7.28
 */
MediaViewer::MediaViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MediaViewer)
    , m_deleteDialog(NULL)
    , m_scale(INITSCALE)
    , m_scaleStep(SCALESTEP)
    , m_minScale(MINSCALE)
    , m_maxScale(MAXSCALE)
    , m_videoPause(true)
    , m_displayValue(0)
    , m_videoInterval(10*1000)
    , m_active(true)
{
    ui->setupUi(this);
    QRect desktop = QApplication::desktop()->geometry();
    desktop.moveTo(QPoint(0, 0));
    setGeometry(desktop); //全屏显示

    m_pSwipeView = new VSSwipeView(ui->centralWidget);
    m_pSwipeView->lower();

    connect(m_pSwipeView, SIGNAL(sigSwitchNext()), SLOT(switchNext()));
    connect(m_pSwipeView, SIGNAL(sigSwitchPrev()), SLOT(switchPrevious()));
    connect(m_pSwipeView, SIGNAL(imageClicked()), SLOT(onImageClicked()));
    connect(m_pSwipeView, SIGNAL(imageDraged()), SLOT(onImageDraged()));
    connect(this, SIGNAL(firstImage()), m_pSwipeView, SLOT(slotFirstImage()));
    connect(this, SIGNAL(lastImage()), m_pSwipeView, SLOT(slotLastImage()));
    connect(this, SIGNAL(oneImage()), m_pSwipeView, SLOT(slotOneImage()));
    connect(this, SIGNAL(resetSwitchEnable()), m_pSwipeView, SLOT(slotResetSwitchEnable()));
    connect(this, SIGNAL(currentImageZoomIn()), m_pSwipeView, SLOT(slotCurrentImageZoomIn()));
    connect(this, SIGNAL(currentImageNoneZoom()), m_pSwipeView, SLOT(slotCurrentImageNoneZoom()));

    ui->label_display->setAlignment(Qt::AlignHCenter);
    ui->label_left->setAlignment(Qt::AlignHCenter);

    m_dispayTimer = new QTimer(this);
    m_dispayTimer->setInterval(1);
    connect(m_dispayTimer, SIGNAL(timeout()), SLOT(slotVideoDisplay()));

    m_pAutoHideTimer = new QTimer;
    m_pAutoHideTimer->setInterval(10000);
    m_pAutoHideTimer->setSingleShot(true);
    connect(m_pAutoHideTimer, SIGNAL(timeout()), SLOT(enterHideState()));
}

MediaViewer::~MediaViewer()
{
    DELETE(m_pSwipeView);
    delete ui;
}

//第一次点击进入多媒体查看界面，需要设置当前文件和文件列表（包含所有的多媒体文件）
void MediaViewer::setCurrentFile(QFileInfo file, QFileInfoList fileList)
{
    m_currentFile = file;

    m_fileList.clear();
    m_fileList = fileList;

    updateImagePresent();
}

/*状态改变事件处理函数(语言改变事件处理在这里)*/
void MediaViewer::changeEvent(QEvent*event)
{
    switch (event->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        QWidget::changeEvent(event);
        break;
    }
}

void MediaViewer::mousePressEvent(QMouseEvent *event)
{
    m_pAutoHideTimer->stop(); //隐藏计时器停止
    QWidget::mousePressEvent(event);
}

void MediaViewer::mouseReleaseEvent(QMouseEvent*event)
{
    //图片模式，显示状态和隐藏模式切换
    if(m_image)
    {
        updateButtonStatus();
    }

    if(!m_active)
    {
        m_pAutoHideTimer->start(); //鼠标释放，隐藏计时器开启
    }

    QWidget::mouseReleaseEvent(event);
}

//切换完成后，需要更新,只将当前文件和其前后的文件添加到滑动界面以供滑动
void MediaViewer::updateImagePresent()
{
    updateMediaState();

    if(m_currentFile != m_fileList.first() && m_currentFile != m_fileList.last())
    {
        emit resetSwitchEnable();

        int index = m_fileList.indexOf(m_currentFile);
        QFileInfo prevFile = m_fileList.value(index-1);
        QFileInfo nextFile = m_fileList.value(index+1);
        m_pSwipeView->setFile(prevFile, m_currentFile, nextFile);
    }
    else if(m_currentFile == m_fileList.first() && m_fileList.count() > 1)
    {
        //当前文件是第一个，则只需要添加当前文件和其后一个（2个文件），而且此时滑动界面不支持向前切换
        emit firstImage();

        QFileInfo prevFile = m_fileList[1];
        QFileInfo nextFile = m_fileList[2];
        m_pSwipeView->setFile(m_currentFile, prevFile, nextFile);
    }
    else if(m_currentFile == m_fileList.last())
    {
        //当前文件是最后一个，则只需要添加当前文件和其前一个（2个），而且此时滑动界面不支持向后切换
        emit lastImage();

        int index = m_fileList.indexOf(m_currentFile);
        QFileInfo prevFile = m_fileList.value(index-2);
        QFileInfo nextFile = m_fileList.value(index-1);
        m_pSwipeView->setFile(prevFile, nextFile, m_currentFile);
    }
    else if(1 == m_fileList.count())
    {
        emit oneImage();

        QFileInfo *file1 = NULL;
        QFileInfo *file2 = NULL;
        m_pSwipeView->setFile(*file1, *file2, m_currentFile);
    }
}

//隐藏状态和显示状态切换
void MediaViewer::updateButtonStatus()
{
    m_active = !m_active;
    if(m_active)
    {
        showFunctionButtons();
    }
    else
    {
        hideFunctionButtons();
    }
}

//隐藏状态，隐藏功能按钮
void MediaViewer::hideFunctionButtons()
{
    ui->toolbar->hide();
    //图片模式相比较视频模式，多了缩放功能按钮
    if(m_image)
    {
        ui->zoombar->hide();
    }
}

//显示状态，显示功能按钮
void MediaViewer::showFunctionButtons()
{
    ui->toolbar->show();
    //图片模式相比较视频模式，多了缩放功能按钮
    if(m_image)
    {
        ui->zoombar->show();
    }
}

//更新界面时，要根据当前的文件类型选择显示按钮的类型（StackedWidget提供切换）
void MediaViewer::updateMediaState()
{
    if(m_currentFile.fileName().endsWith(".jpg"))
    {
        ui->toolbar->setCurrentWidget(ui->toolbarimage);
        m_image = true;  //当前查看的文件类型是图片
    }
    else
    {
        ui->zoombar->hide();
        ui->toolbar->setCurrentWidget(ui->toolbarvideo);
        m_image = false; //当前查看的文件类型是视频

        m_displayValue = 0; //当前的播放时间
        ui->videoSlider->setValue(0); //播放进度条的播放时间
        m_videoPause = true; //当前处于暂停状态
        ui->button_display->setStyleSheet(QString("border-image: url(:/res/icons/ic_display.png);"));
        m_dispayTimer->stop(); //控制进度条的计时器未开始
        m_videoInterval = 10*1000; //需要获取视频文件的总时间，当前手动设置？？？？？
        ui->videoSlider->setRange(0, m_videoInterval); //设置进度条范围
        ui->videoSlider->setPageStep(1); //设置进度条步进值

        if(m_pAutoHideTimer->isActive())
            m_pAutoHideTimer->stop();
        m_pAutoHideTimer->start();  //开启自动隐藏工具按钮的计时器
    }
    m_active = true;    //工具按钮处于显示状态
    showFunctionButtons();
}

//播放自动停止
void MediaViewer::videoAutoStop()
{
    m_videoPause = true;
    ui->button_display->setStyleSheet(QString("border-image: url(:/res/icons/ic_display.png);"));
    m_dispayTimer->stop();
}

void MediaViewer::showDeleteDialog()
{
    if(!m_deleteDialog)
    {
        m_deleteDialog = new DeleteDialog(this);
        connect(m_deleteDialog, SIGNAL(sigAccept()), SLOT(deleteAccept()));
        connect(m_deleteDialog, SIGNAL(sigReject()), SLOT(deleteReject()));
        m_deleteDialog->setGeometry(QRect(0,0,width(),height()));
        m_deleteDialog->show();
    }
}

//点击图片，功能按钮显示状态切换
void MediaViewer::onImageClicked()
{
    updateButtonStatus();
}

//切换图片过程中，功能按钮隐藏
void MediaViewer::onImageDraged()
{
    m_active = false;
    hideFunctionButtons();
}

void MediaViewer::on_gobackButton_clicked()
{
   emit goback();
}

//放大
void MediaViewer::on_zoominButton_clicked()
{
    if(m_scale < m_maxScale)
    {
       m_scale += m_scaleStep;
       if(m_scale >= m_maxScale)
       {
           //放大到最大，放大按钮无效
           m_scale = m_maxScale;
           ui->zoominButton->setEnabled(false);
           ui->zoomoutButton->setEnabled(true);
       }
       else
       {
           ui->zoominButton->setEnabled(true);
           ui->zoomoutButton->setEnabled(true);
       }
       m_pSwipeView->zoomIn(m_scale); //图片查看器将图片放大到响应的比例
       emit currentImageZoomIn();  //放大状态，滑动界面不支持切换
    }
}

//缩小
void MediaViewer::on_zoomoutButton_clicked()
{
    if(m_scale > m_minScale)
    {
        m_scale -= m_scaleStep;
        if(m_scale <= m_minScale)
        {
            m_scale = m_minScale;
            ui->zoomoutButton->setEnabled(false);
            m_pSwipeView->zoomReset();
        }
        else
        {
            ui->zoominButton->setEnabled(true);
            ui->zoomoutButton->setEnabled(true);
        }
        m_pSwipeView->zoomOut(m_scale);
    }
    if(m_scale == m_minScale)
    {
        emit currentImageNoneZoom(); //缩放还原，滑动界面支持切换
    }
}

//删除图片槽函数
void MediaViewer::on_deleteButton_clicked()
{
    m_active = false;
    hideFunctionButtons();

    showDeleteDialog();
}

//删除图片槽函数
void MediaViewer::deleteAccept()
{
    m_deleteDialog->hide();
    DELETE(m_deleteDialog); //回收删除对话框内存
    m_active = false;
    hideFunctionButtons();
    emit itemDeleted(m_currentFile);  //发送文件删除信号，并携带当前文件信息

    //将当前文件从文件列表中删除，并更新当前文件
    if(m_fileList.count() <= 1)
    {
        //特殊情况，文件列表被删空
    }
    else if(m_currentFile == m_fileList.last())
    {
        //删除的是最后一张
        m_fileList.removeOne(m_currentFile);
        m_currentFile = m_fileList.last();
    }
    else
    {
        //删除的是其他情况下的任意一张
        int index = m_fileList.indexOf(m_currentFile);
        m_fileList.removeOne(m_currentFile);
        m_currentFile = m_fileList.value(index);
    }
    updateImagePresent(); //更新显示
}

//删除操作取消
void MediaViewer::deleteReject()
{
    m_deleteDialog->hide();
    DELETE(m_deleteDialog);
    showFunctionButtons();
}

//切换到前一张
void MediaViewer::switchPrevious()
{
    int index = m_fileList.indexOf(m_currentFile);
    m_currentFile = m_fileList.value(index-1);
    updateImagePresent();
}

//切换到后一张
void MediaViewer::switchNext()
{
    int index = m_fileList.indexOf(m_currentFile);
    m_currentFile = m_fileList.value(index+1);
    updateImagePresent();
}

//删除视频槽函数
void MediaViewer::on_button_delete_clicked()
{
    //delete current video
    m_videoPause = true;
    ui->button_display->setStyleSheet(QString("border-image: url(:/res/icons/ic_display.png);"));
    m_dispayTimer->stop();

    showDeleteDialog();
}

void MediaViewer::on_button_back_clicked()
{
    IpcInstance->stopPlayRecord();
    emit goback();
}

//播放/暂停视频槽函数
void MediaViewer::on_button_display_clicked()
{
    m_videoPause = !m_videoPause;
    if(m_videoPause)
    {
        //暂停
        ui->button_display->setStyleSheet(QString("border-image: url(:/res/icons/ic_display.png);"));
        m_dispayTimer->stop();
    }
    else
    {
        //播放
        if(IpcInstance->connectToServer() < 0)
            qDebug() << "connect to server failed!";
        IpcInstance->startPlayRecord(MEDIA_FOLDER, m_currentFile.fileName());
        if(m_displayValue == m_videoInterval)
        {
            //播放完状态，自动从头开始播放
            m_displayValue = 0;
            ui->label_display->setText(QString::number(m_displayValue));
            ui->label_left->setText(QString::number(m_videoInterval - m_displayValue));
        }
        ui->button_display->setStyleSheet(QString("border-image: url(:/res/icons/ic_display_2.png);"));
        m_dispayTimer->start(); //进度条计时器开启
    }
}

//快退槽函数
void MediaViewer::on_button_backward_clicked()
{
    if(m_displayValue < m_videoInterval && m_displayValue >= 1000)
    {
        m_displayValue -= 1000;
        ui->videoSlider->setValue(m_displayValue);
        ui->label_display->setText(QString::number(m_displayValue));
        ui->label_left->setText(QString::number(m_videoInterval - m_displayValue));
    }
}

//快进槽函数
void MediaViewer::on_button_forward_clicked()
{
    if(m_displayValue < m_videoInterval)
    {
        m_displayValue += 1000;
        ui->videoSlider->setValue(m_displayValue);
        ui->label_display->setText(QString::number(m_displayValue));
        ui->label_left->setText(QString::number(m_videoInterval - m_displayValue));
    }
}

//播放视频帧，视频进度条控制
void MediaViewer::slotVideoDisplay()
{
    if(m_displayValue < m_videoInterval)
    {
        m_displayValue += 1;
        ui->videoSlider->setValue(m_displayValue);
        ui->label_display->setText(QString::number(m_displayValue));
        ui->label_left->setText(QString::number(m_videoInterval - m_displayValue));
    }
    else
    {
        videoAutoStop();
    }
}

//视频跳转槽函数
void MediaViewer::on_videoSlider_valueChanged(int value)
{
    m_displayValue = value;
    ui->label_display->setText(QString::number(m_displayValue));
    ui->label_left->setText(QString::number(m_videoInterval - m_displayValue));
    //控制服务器视频跳转
    //
}

void MediaViewer::enterHideState()
{
    m_active = false; //进入隐藏状态，功能按钮隐藏
    hideFunctionButtons();
}
