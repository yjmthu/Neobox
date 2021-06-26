//#if defined(_MSC_VER) && (_MSC_VER >= 1600)
//# pragma execution_character_set("utf-8")
//#endif
#include "form.h"
#include <QScreen>
#include <QDateTime>
//#include <QTextCodec>
#include <QSharedMemory>
#include <QStyleFactory>
//#include <shellapi.h>

//void start_log(bool clean)                                                      //在日志文件第一行写入当前时间
//{
//	QDateTime dateTime(QDateTime::currentDateTime());
//	QString datetime_str = dateTime.toString("yyyy-MM-dd hh:mm:ss");
//	FuncBox::write_log(datetime_str, clean);
//}

int main(int argc, char* argv[])
{
	QSharedMemory shared_memory;
	shared_memory.setKey(QString("speed_box"));
	if (shared_memory.attach())                                                 //防止多次打开
		if ((argc == 1) || (QString(argv[1]).compare("restart")))
			return 0;
	if (shared_memory.create(1) || (argc > 1))                                      //创建1byte的共享内存段
	{
        //start_log(argc == 1);                                                  //开始记录日志
#if (QT_VERSION > QT_VERSION_CHECK(5,6,0))&&(QT_VERSION < QT_VERSION_CHECK(6,0,0))
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);           //适应高分辨率屏幕
#endif
		QApplication a(argc, argv);                                            //创建app
        QApplication::setStyle(QStyleFactory::create("Fusion"));
//        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//        QTextCodec::setCodecForLocale(codec);
		a.setQuitOnLastWindowClosed(false);                                    //防止QFileDialog被当成最主窗口导致程序结束
		QScreen* screen = QGuiApplication::primaryScreen();                    //获取屏幕分辨率
		QRect geo = screen->geometry();
		VarBox.SCREEN_WIDTH = geo.width(); VarBox.SCREEN_HEIGHT = geo.height();
        Form w;                                                                //创建悬浮窗，并且传入屏幕数据和当前路径。
        APPBARDATA abd;
        memset(&abd, 0, sizeof(abd));
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = (HWND)w.winId();
        abd.uCallbackMessage = MSG_APPBAR_MSGID;
        SHAppBarMessage(ABM_NEW, &abd);
		w.show();                                                              //显示悬浮窗
		int e = a.exec();
		if (e == RETCODE_RESTART)                                              //如果收到重启常数
		{
			QProcess::startDetached(a.applicationFilePath(), QStringList("restart"));   //重启程序
		}
		return 0;
	}
	return 0;
}
