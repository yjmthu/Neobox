#ifndef FUNCBOX_H
#define FUNCBOX_H

#include <QApplication>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>

//#define cout qDebug().noquote()
#define D(a) FuncBox::write_log(a)
#define A(a) {FuncBox::write_log(#a); qDebug() << #a; a}
#define RETCODE_RESTART 773       //重启常数，双击时界面会返回这个常数实现整个程序重新启动。
#define MSG_APPBAR_MSGID 2731

#if defined(Q_OS_WIN32)
#include <winsock2.h>
#include <iphlpapi.h>
#include <oleacc.h>
#include <oaidl.h>
#include <windows.h>
#include <WinInet.h>
//#include <winhttp.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#endif

typedef enum _PAPER_TYPES
{
	Latest = 0, Hot = 1, Nature = 2, Anime = 3, Simple = 4, Random = 5, Bing = 6, Wallpapers = 7, Native = 8, Advance = 9
} PAPER_TYPE;

typedef enum _COLOR_THEME
{
    ClassicWhite = 0, SomeGray = 1, TsinghuaPurple = 2, PekinRed = 3
} COLOR_THEME;

typedef struct _VAR_BOX
{
	static QString FAMILY_NAMES[10];
    static const char *PaperTypes[10];         //九种壁纸类型
	static QString FAMILY_PATH;                //壁纸文件夹的上一级目录
	static PAPER_TYPE PaperType;               //当下正在使用的壁纸类型
    static COLOR_THEME cur_theme;
    //static bool FULL_SCRENN;                   //是否存在全屏窗口
	static bool RUN_APP;                       //app运行状态
	static bool RUN_NET_CHECK;                 //是否停止网速检测

	static bool AutoChange;                    //当下是否已经启用自动更换壁纸
	static short PageNum;                      //当下使用的壁纸页面（每页120张图片）
	static QString NativeDir;                  //当下正在使用的用户本地壁纸文件夹
	static QString UserCommand;                //当下正在使用的用户高级命令
	static QString PATH_TO_OPEN;               //要打开的文件夹

	static int SCREEN_WIDTH;                   //屏幕宽度
	static int SCREEN_HEIGHT;                  //屏幕高度

	static bool HAVE_APP_RIGHT;
	static bool ENABLE_TRANSLATER;
	static QString APP_ID;
	static QString PASS_WORD;
} VAR_BOX;

static VAR_BOX VarBox;

namespace FuncBox {

	bool check_or_make_dir(QString);
	QString get_son_dir(QString);
	QString get_son_dir(const char[]);
	QString get_ini_path();
	QString get_wal_path();
	QString get_dat_path();
	QString get_pic_path(short);
	void save_the_open_path();
	bool build_init_files();                                           //创建程序所需文件
	void write_log(QString, bool clean = false);                       //记录日志
	bool isOnline(bool);                                               //检查是否有网络连接，布尔值代表是否保持检测 30 秒
//	bool getReplyData(QNetworkAccessManager*, QUrl, QString*);         //获取对url的请求结果，写入到QString
//	bool getReplyData(QNetworkAccessManager*, QUrl, QByteArray*);      //获取对url的请求结果，写入到QByteArry
	QString runCommand(QString, QStringList, short i = 0);             //执行命令，返回第i行结果，默认返回空字符串
    bool getWebCode(const WCHAR*, QString&);
    bool getBingCode(QString &code);
    bool downloadImage(QString, QString);
    bool getTransCode(const WCHAR* url,QString &outcome);

}

#endif // FUNCBOX_H
