#include "filebrowser.h"
#include "ui_filebrowser.h"
#include <QDir>
#include <QDirIterator>
#include <QPainter>
#include <QDebug>
#include <QItemSelectionModel>
#include <QTime>
#include <QStackedWidget>
#include <QScrollBar>
#include <QState>
#include <QImage>
#include "vsglobal.h"
#include "mp4reader.h"

#define CAMERA_WIDTH  960
#define CAMERA_HEIGHT 720

#ifndef __arm__
#define H_CENTRAL  560
#define W_GRIDSIZE 288
#define H_GRIDSIZE 168
#define SPACE 20
#else
#define H_CENTRAL  280
#define W_GRIDSIZE 144
#define H_GRIDSIZE 96
#define SPACE 8
#endif

FileBrowser::FileBrowser(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileBrowser)
    , m_scrollArea(NULL)
    , m_pMediaViewer(NULL)
    , m_deleteDialog(NULL)
    , m_videoReader(NULL)
    , m_isImageOnly(false)
    , m_isVideoOnly(false)
    , m_bMousePressed(false)
    , m_IconsLoadFlag(false)
    , m_scroll(false)
{
    ui->setupUi(this);

#ifdef WIN32
    m_path = tr("F:/Multimedia/meidas/");
#else
    m_path = MEDIA_FOLDER;
#endif
    m_dir.setPath(m_path);

    m_scrollArea = new QScrollArea(ui->centralWidget);
    setScrollBarStyle();

    m_pFileListWidget=new VSFileListWidget;
    m_pFileListWidget->setViewMode(QListWidget::IconMode);
    m_pFileListWidget->setUniformItemSizes(true);
    m_pFileListWidget->setResizeMode(QListWidget::Fixed);
    m_pFileListWidget->setFlow(QListWidget::LeftToRight);
    m_pFileListWidget->setMovement(QListWidget::Static);
    m_pFileListWidget->setSpacing(SPACE);
    connect(m_pFileListWidget,SIGNAL(selectionModeChanged(QAbstractItemView::SelectionMode))
            ,SLOT(selectionModeChanged(QAbstractItemView::SelectionMode)));
    connect(m_pFileListWidget, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));

    m_scrollArea->setWidget(m_pFileListWidget);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_pIconLoader = new IconLoader;
    connect(m_pIconLoader, SIGNAL(iconLoaded(const QString)), SLOT(iconLoaded(const QString)));

    updateFileIcons();

    m_pBottomBarStateMachine = new QStateMachine;
    m_pNormalState = new QState(m_pBottomBarStateMachine);
    m_pDeleteState = new QState(m_pBottomBarStateMachine);

    connect(m_pNormalState, SIGNAL(entered()), SLOT(enterNormalState()));

    m_pNormalState->assignProperty(ui->button_home,"styleSheet","border-image:url(://res/icons/ic_home_unchecked.png)");
    m_pNormalState->assignProperty(ui->button_home,"visible","true");
    m_pNormalState->addTransition(this,SIGNAL(enterDeleteMode()),m_pDeleteState);

    m_pDeleteState->assignProperty(ui->button_home,"visible","false");
    m_pDeleteState->assignProperty(ui->button_image,"styleSheet","border-image:url(://res/icons/ic_delete_2.png)");
    m_pDeleteState->assignProperty(ui->button_video,"styleSheet","border-image:url(://res/icons/ic_cancel_2.png)");
    m_pDeleteState->addTransition(this,SIGNAL(enterNormalMode()),m_pNormalState);

    m_pBottomBarStateMachine->setInitialState(m_pNormalState);
    m_pBottomBarStateMachine->start();

}

FileBrowser::~FileBrowser()
{
    delete ui;
}

void FileBrowser::showEvent(QShowEvent *event)
{
    m_pFileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModeChanged(m_pFileListWidget->selectionMode());

    QWidget::showEvent(event);
}

void FileBrowser::resizeEvent(QResizeEvent *event)
{
    m_scrollArea->setFixedSize(ui->centralWidget->width(), ui->centralWidget->height());

    updateListHeight();

    m_pFileListWidget->setStyleSheet(QString("border:0px solid transparent; color: gray;"));

    QWidget::resizeEvent(event);
}

void FileBrowser::updateListHeight()
{
    quint32 count = m_files.count();
    quint32 rowNum = (0 == count%3) ? (count/3) : (count/3+1);
    qint64 listHeight = rowNum*(H_GRIDSIZE + SPACE);
    if(listHeight < height())
            listHeight = H_CENTRAL + SPACE;
    m_pFileListWidget->setFixedSize(width(), listHeight);
}

void FileBrowser::setScrollBarStyle()
{
    m_scrollArea->setStyleSheet(
               QString("\
               QScrollBar::vertical {\
                  background-color: white;\
                  border-color: white;\
                  width: 10px;\
               }\
               QScrollBar::handle:vertical {\
                  border-image: url(:/res/icons/ic_scrollbar_handle.png);\
                  min-height: 60px;\
                  width: 10px;\
               }\
               QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {\
                  subcontrol-origin: margin;\
                  height: 30px;\
                  background-color: transparent;\
               }\
               QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                 background-color: transparent;\
                 border:0px solid transparent;\
               }\
               QScrollArea {\
                 border:0px solid transparent;\
               }\
            "));
}

void FileBrowser::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_bMousePressed)
    {
        return;
    }

    QPoint currentPt = event->pos();

    int dist = m_pressPosition.y() - currentPt.y();

    m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() + dist);

    m_pressPosition = currentPt;

    m_scroll = true;

    QWidget::mouseMoveEvent(event);
}

void FileBrowser::mousePressEvent(QMouseEvent *event)
{
    m_bMousePressed = true;
    m_pressPosition = event->pos();

    QWidget::mousePressEvent(event);
}

void FileBrowser::mouseReleaseEvent(QMouseEvent *event)
{
    m_bMousePressed = false;
    m_scroll = false;

    m_pressPosition.setX(0);
    m_pressPosition.setY(0);

    QWidget::mouseReleaseEvent(event);
}

/*!
 * \brief FileBrowser::selectionModeChanged 选择模式发生改变的处理函数
 * \param mode 单选/多选两种模式
 */
void FileBrowser::selectionModeChanged(QAbstractItemView::SelectionMode mode)
{
    QMap<QListWidgetItem *, ItemLabel *>::const_iterator it = m_itemMap.constBegin();
    while(it != m_itemMap.constEnd())
    {
        drawPixmapOnIcon(it.key());
        ++it;
    }

    if(QAbstractItemView::SingleSelection == mode)
    {
        m_pFileListWidget->clearSelection();
        emit enterNormalMode();
    }
    else
    {
        emit enterDeleteMode();
    }
}

/*!
 * \brief FileBrowser::itemClicked 文件列表的item被点击的处理函数
 * \param item 被点击的列表项
 */
void FileBrowser::itemClicked(QListWidgetItem *item)
{
    if(m_pFileListWidget->selectionMode() == QAbstractItemView::MultiSelection)
    {
        //多选模式，更新图片的选择标识，并根据当前选择的张数判断删除按钮的使能
        drawPixmapOnIcon(item);
        if(m_pFileListWidget->selectedItems().size() == 0)
            ui->button_image->setEnabled(false);
        else
            ui->button_image->setEnabled(true);
    }
    else
    {
        if(m_scroll == true)
        {
            return;
        }

        //单选模式，创建多媒体文件浏览类对象
        if(QStackedWidget *pParent = dynamic_cast<QStackedWidget*>(this->parent()))
        {
            if(!m_pMediaViewer)
            {
                m_pMediaViewer = new MediaViewer();
            }

            connect(m_pMediaViewer, SIGNAL(itemDeleted(QFileInfo)), SLOT(itemDeleted(QFileInfo)));
            connect(m_pMediaViewer, SIGNAL(goback()), SLOT(showFileBrowser()));

            pParent->addWidget(m_pMediaViewer);
            QFileInfo file(m_path+item->text());
            m_pMediaViewer->setCurrentFile(file, m_files);

            pParent->setCurrentWidget(m_pMediaViewer);

        }
    }
}

/*!
 * \brief FileBrowser::updateFileIcons
 */
void FileBrowser::updateFileIcons()
{
    getFiles();
    m_pIconLoader->setFiles(m_files);
}

/* loadNewMedia: 对外接口，用于拍照完成后，界面更新最新文件 */
/* name: 文件名(不包括路径) */
void FileBrowser::loadNewMedia(QString name)
{
    if(m_files.contains(QFileInfo(m_path+name)))
        return;

    m_files.append(QFileInfo(m_path+name)); //文件列表添加最新文件
    iconLoaded(name); //绘制最新文件的图标
}

/*!
 * \brief FileBrowser::getFiles 获取本地所有文件并更新文件列表
 */
void FileBrowser::getFiles()
{
    QStringList filters;
    filters << "*.jpg" << "*.mp4";
    QDirIterator it(m_dir.absolutePath(), filters, QDir::Files | QDir::NoDotAndDotDot);

    QList<QString> names;
    while(it.hasNext())
    {
        it.next();
        names.append(it.fileInfo().fileName());
    }
    std::sort(names.begin(),names.end());

    m_files.clear();
    foreach (QString name, names)
    {
        QFileInfo file= QFileInfo(m_path+name);
        m_files.append(file);
    }
    updateListHeight(); //更新滚动区高度
}

/*!
 * \brief FileBrowser::iconLoaded 根据文件名绘制图标
 * \param name 文件名
 */
void FileBrowser::iconLoaded(const QString name)
{
    QPixmap icon;
    if(name.endsWith(".jpg") || name.endsWith(".png"))
    {
        icon = QPixmap(m_path+name);  //图片文件，绘制
    }
    else if(name.endsWith(".mp4"))
    {
        //视频文件，提取视频帧
        int size = 0;
        VideoReader->setFile(m_path+name, size);

        char pFrame[size];
        VideoReader->readFrame(1, pFrame, size);

        QFile testfile(m_path + name + QString(".jpg"));
        if(!testfile.open(QIODevice::WriteOnly))
        {
            qDebug() << "open test.jpg error!";
            return;
        }

        QDataStream writeStream(&testfile);
        writeStream<<pFrame;
        testfile.close();

        qDebug() << "111111111111111111111111" << size;

        QImage image((uchar *)pFrame, W_GRIDSIZE, H_GRIDSIZE, QImage::Format_RGB888);
        icon = QPixmap::fromImage(image);
    }

    QListWidgetItem *item = new QListWidgetItem(name);
    item->setSizeHint(QSize(W_GRIDSIZE, H_GRIDSIZE));
    ItemLabel *label = new ItemLabel(icon, name, QSize(W_GRIDSIZE, H_GRIDSIZE));
    m_pFileListWidget->addItem(item);
    m_pFileListWidget->setItemWidget(item, label);
    m_itemMap.insert(item, label);

    drawPixmapOnIcon(item);
}

/*!
 * \brief FileBrowser::drawPixmapOnIcon
 * 单选模式下，所有的图片都没有选中标识
 * 多选模式下，被选中的图片带有选中标识
 */
void FileBrowser::drawPixmapOnIcon(QListWidgetItem *item)
{
    ItemLabel *label = m_itemMap[item];
    if(m_pFileListWidget->selectionMode() != QAbstractItemView::MultiSelection)
    {
        label->unSelected();
    }
    else
    {
        if(item->isSelected())
        {
            label->setSelected();
        }
        else
        {
            label->unSelected();
        }
    }
}

/*!
 * \brief FileBrowser::on_button_image_clicked
 * 图片/删除按钮点击处理函数
*/
void FileBrowser::on_button_image_clicked()
{
    if(m_pFileListWidget->selectionMode() == QAbstractItemView::SingleSelection)
    {
        //单选模式，进入/退出只读图片模式
        ui->button_video->setStyleSheet(QString("border-image: url(:/res/icons/ic_video_unchecked.png);"));
        m_isVideoOnly = false;
        showAllItems();
        if(false == m_isImageOnly)
        {
            QMap<QListWidgetItem *, ItemLabel *>::const_iterator it = m_itemMap.constBegin();
            while(it != m_itemMap.constEnd())
            {
                if(it.key()->text().endsWith(".mp4"))
                {
                    it.key()->setHidden(true);
                }
                ++it;
            }

            m_isImageOnly = true;
            ui->button_image->setStyleSheet(QString("border-image: url(:/res/icons/ic_pic_checked.png);"));
        }
        else
        {
            QMap<QListWidgetItem *, ItemLabel *>::const_iterator it = m_itemMap.constBegin();
            while(it != m_itemMap.constEnd())
            {
                it.key()->setHidden(false);
                ++it;
            }

            m_isImageOnly = false;
            ui->button_image->setStyleSheet(QString("border-image: url(:/res/icons/ic_pic_unchecked.png);"));
        }
    }
    else
    {
        //多选模式，作为删除按钮使用，创建删除对话框
        if(!m_deleteDialog)
        {
            m_deleteDialog = new DeleteDialog(this);
        }
        connect(m_deleteDialog, SIGNAL(sigAccept()), SLOT(acceptDelete()));
        connect(m_deleteDialog, SIGNAL(sigReject()), SLOT(acceptReject()));
        m_deleteDialog->setGeometry(0,0,this->width(),this->height());
        m_deleteDialog->show();
    }
}

/*!
 * \brief FileBrowser::on_button_video_clicked
 * 视频/退出按钮点击处理函数
 */
void FileBrowser::on_button_video_clicked()
{
    if(m_pFileListWidget->selectionMode() == QAbstractItemView::SingleSelection)
    {
        //单选模式，进入/退出只读视频模式
        ui->button_image->setStyleSheet(QString("border-image: url(:/res/icons/ic_pic_unchecked.png);"));
        m_isImageOnly = false;
        showAllItems();
        if(false == m_isVideoOnly)
        {
            QMap<QListWidgetItem *, ItemLabel *>::const_iterator it = m_itemMap.constBegin();
            while(it != m_itemMap.constEnd())
            {
                if(it.key()->text().endsWith(".jpg"))
                {
                    it.key()->setHidden(true);
                }
                ++it;
            }

            m_isVideoOnly = true;
            ui->button_video->setStyleSheet(QString("border-image: url(:/res/icons/ic_video_checked.png);"));
        }
        else
        {
            QMap<QListWidgetItem *, ItemLabel *>::const_iterator it = m_itemMap.constBegin();
            while(it != m_itemMap.constEnd())
            {
                it.key()->setHidden(false);
                ++it;
            }

            m_isVideoOnly = false;
            ui->button_video->setStyleSheet(QString("border-image: url(:/res/icons/ic_video_unchecked.png);"));
        }
    }
    else if(m_pFileListWidget->selectionMode() == QAbstractItemView::MultiSelection)
    {
        //多选模式，作为退出按钮使用，退出多选模式
        m_pFileListWidget->switchMode();
    }
}

/*开始添加图标*/
void FileBrowser::StartIconLoad()
{
    if(!m_pIconLoader)
    {
        return;
    }

    /*如果未加载过图标，则开始加载图标*/
    if(!m_IconsLoadFlag)
    {
        m_pIconLoader->start();
        m_IconsLoadFlag = true;
    }
}

/*状态改变事件处理函数(语言改变事件处理在这里)*/
void FileBrowser::changeEvent(QEvent*event)
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

/*!
 * \brief FileBrowser::showFileBrowser
 * 从文件查看界面返回到本界面
 */
void FileBrowser::showFileBrowser()
{
    if(QStackedWidget* pParent = dynamic_cast<QStackedWidget *>(this->parent()))
    {
        if(m_pMediaViewer)
        {
            pParent->removeWidget(m_pMediaViewer);
            DELETE(m_pMediaViewer);
        }
        pParent->setCurrentWidget(this);
    }
}

/*!
 * \brief FileBrowser::itemDeleted 处理文件删除的槽函数
 * \param file 被删除的文件名
 */
void FileBrowser::itemDeleted(QFileInfo file)
{
    //遍哈希表，将对应的项删除
    QMap<QListWidgetItem *, ItemLabel *>::iterator it = m_itemMap.begin();

    while(it != m_itemMap.end())
    {
        if(it.key()->text() == file.fileName())
        {
            int row = m_pFileListWidget->row(it.key());
            m_pFileListWidget->takeItem(row);

            it = m_itemMap.erase(it);
        }
        else
        {
            ++it;
        }
    }

    QFile::remove(file.absoluteFilePath()); //删除本地文件
    m_files.removeOne(file); //删除文件列表中的项
    updateListHeight(); //更新滚动区域的高度
    emit fileDeleted(); //发送文件删除信号
}

/*!
 * \brief FileBrowser::showAllItems
 * 显示所有图标
 */
void FileBrowser::showAllItems()
{
    QMap<QListWidgetItem *, ItemLabel *>::const_iterator it = m_itemMap.constBegin();

    while(it != m_itemMap.constEnd())
    {
        it.key()->setHidden(false);
        ++it;
    }
}

/* 对外接口，用于获取最新文件 */
QString FileBrowser::getLastFile()
{
    if(m_files.count())
    {
        return m_files.last().absoluteFilePath();
    }
    return QString::null;
}

/* 对外接口，用于获取所有文件名 */
QList<QString> FileBrowser::getFileNames()
{
    QList<QString> names;
    foreach (QFileInfo file, m_files) {
        names.append(file.fileName());
    }
    return names;
}

void FileBrowser::on_button_home_released()
{
    ui->button_home->setStyleSheet(QString("border-image: url(:/res/icons/ic_home_unchecked.png);"));
    emit home();
}

void FileBrowser::on_button_home_pressed()
{
    ui->button_home->setStyleSheet(QString("border-image: url(:/res/icons/ic_home_checked.png);"));
}

void FileBrowser::enterNormalState()
{
    if(m_isImageOnly)
    {
        m_pNormalState->assignProperty(ui->button_image,"styleSheet","border-image:url(://res/icons/ic_pic_checked.png)");
        m_pNormalState->assignProperty(ui->button_video,"styleSheet","border-image:url(://res/icons/ic_video_unchecked.png)");
    }
    else if(m_isVideoOnly)
    {
        m_pNormalState->assignProperty(ui->button_image,"styleSheet","border-image:url(://res/icons/ic_pic_unchecked.png)");
        m_pNormalState->assignProperty(ui->button_video,"styleSheet","border-image:url(://res/icons/ic_video_checked.png)");
    }
    else
    {
        m_pNormalState->assignProperty(ui->button_image,"styleSheet","border-image:url(://res/icons/ic_pic_unchecked.png)");
        m_pNormalState->assignProperty(ui->button_video,"styleSheet","border-image:url(://res/icons/ic_video_unchecked.png)");
    }
}

/*!
 * \brief FileBrowser::acceptDelete
 * 删除处理函数
 */
void FileBrowser::acceptDelete()
{
    m_deleteDialog->hide();
    DELETE(m_deleteDialog);
    QList<QListWidgetItem* >list = m_pFileListWidget->selectedItems();
    if(!list.count())
        return;

    //两层循环，时间复杂度O(n2)
    foreach (QListWidgetItem *item, list)
    {
        QFileInfo file(m_path+item->text());

        //删除哈希表中的对应项
        QMap<QListWidgetItem *, ItemLabel *>::iterator it = m_itemMap.begin();
        while(it != m_itemMap.end())
        {
            if(it.key() == item)
            {
                int row = m_pFileListWidget->row(it.key());
                m_pFileListWidget->takeItem(row);

                it = m_itemMap.erase(it);
            }
            else
            {
                ++it;
            }
        }

        QFile::remove(file.absoluteFilePath());  //删除本地文件
        m_files.removeOne(file); //文件列表中删除项
    }
    emit fileDeleted();  //删除完成后发送删除信号通知主界面
    updateListHeight(); //删除完成后更新滚动区高度
    m_pFileListWidget->switchMode(); //删除完成后退出多选模式
}

void FileBrowser::acceptReject()
{
    m_deleteDialog->hide();
    DELETE(m_deleteDialog);
}
