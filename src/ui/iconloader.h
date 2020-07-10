#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QObject>
#include <QVector>
#include <QFileInfoList>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QPixmap>
#include <QIcon>
#include <QTime>
#include <QCoreApplication>
#include <QEventLoop>
#include <QDebug>
#include <QFileInfo>

class IconLoader : public QThread
{
    Q_OBJECT
public:
    explicit IconLoader(QObject *parent = 0);

signals:
    void iconLoaded(const QString fileName);

public slots:
    void setFiles(QFileInfoList files)
    {
        m_files = files;
    }

protected:
    void run();

private:
    QFileInfoList 	m_files;

};

#endif // ICONLOADER_H
