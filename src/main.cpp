#include "vsmainwindow.h"
#include "service/vsdatabase.h"
#include "version.h"
#include "SingleApplication.h"
#include "powersupplyprocess.h"
#include <QTextCodec>
#include <QWSServer>
#include <stdlib.h>
#include "feeddog.h"

void SetTextCode(const char*CodeName);

void loadStyleSheet(QApplication * const app, const QString &fileName);
void loadConfig();

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

    SingleApplication app(argc, argv, "vs31_client");
    if(app.isRunning())
        exit(EXIT_SUCCESS);

//    FeedDog::instance()->start();
    loadStyleSheet(&app, ":/app.qss");

#ifdef __arm__
    QWSServer::setBackground(QBrush(Qt::TransparentMode));//设置QWS服务器背景色为透明
//    QWSServer::setCursorVisible(false);
#endif
    SetTextCode("utf-8");

	loadConfig();

    VSMainWindow window;

#ifdef __arm__
    window.showFullScreen();
#else
    window.move(QPoint(350,150));
    window.resize(960,640);
    window.show();
#endif

    int ret = app.exec();
    return ret;
}

void loadStyleSheet(QApplication * const app, QString const &fileName)
{
    QFile qss(fileName);
    if(qss.open(QFile::ReadOnly))
    {
        app->setStyleSheet(qss.readAll());
        qss.close();
    }
}

void loadConfig()
{
}
	
void SetTextCode(const char * CodeName)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(CodeName));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName(CodeName));
    QTextCodec::setCodecForTr(QTextCodec::codecForName(CodeName));
}
