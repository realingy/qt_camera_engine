#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QWidget>
#include <QFileInfoList>
#include <QStateMachine>
#include "vsfilelistwidget.h"
#include "iconloader.h"
#include "mediaviewer.h"
#include "deletedialog.h"
#include "itemlabel.h"
#include "mp4reader.h"

/*!
 * FileBrowser: 多媒体文件预览界面
 * 2018.7.25
 * 5005965
 */
namespace Ui {
class FileBrowser;
}

class FileBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit FileBrowser(QWidget *parent = 0);
    ~FileBrowser();

    void loadNewMedia(QString name);
    QString getLastFile();
    QList<QString> getFileNames();
    void StartIconLoad();//开始添加图标

protected:
    void changeEvent(QEvent *event);//状态改变事件处理函数(语言改变事件处理在这里)

signals:
    void home();
    void enterDeleteMode();
    void enterNormalMode();
    void fileDeleted();

public slots:
    void iconLoaded(const QString);
    void itemDeleted(QFileInfo );
    void updateFileIcons();
    void selectionModeChanged(QAbstractItemView::SelectionMode mode);
    void itemClicked(QListWidgetItem* item);
    void showFileBrowser();

protected:
    void showEvent(QShowEvent* event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void on_button_image_clicked();
    void on_button_video_clicked();
    void on_button_home_released();
    void on_button_home_pressed();
    void enterNormalState();
    void acceptDelete();
    void acceptReject();

private:
    void getFiles();
    void drawPixmapOnIcon(QListWidgetItem *item);
    void showAllItems();
    void updateListHeight();
    void setScrollBarStyle();

private:
    Ui::FileBrowser         *ui;
    QScrollArea             *m_scrollArea;
    VSFileListWidget        *m_pFileListWidget;
    MediaViewer             *m_pMediaViewer;
    IconLoader              *m_pIconLoader;
    DeleteDialog            *m_deleteDialog;
    MP4Reader               *m_videoReader;
    QStateMachine           *m_pBottomBarStateMachine;
    QState                  *m_pNormalState;
    QState                  *m_pDeleteState;

    QList<QFileInfo> 		 m_files;
    QMap<QListWidgetItem *, ItemLabel *> m_itemMap;  //维护哈希表，方便实现只读模式和控制图标的选中标识

    QPoint      m_pressPosition;
    QString		m_path;
    QDir		m_dir;

    bool	    m_isImageOnly;
    bool		m_isVideoOnly;
    bool        m_bMousePressed;
    bool        m_IconsLoadFlag; //图标加载标志
    bool        m_scroll;
};

#endif // FILEBROWSER_H
