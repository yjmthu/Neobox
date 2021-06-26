#include "menu.h"
#include "form.h"
#include "menuwallpaper.h"

#include <QMessageBox>

Menu::Menu(Form* v) :
	form(v)
{
    D("初始化右键菜单");
    initActions();
	initUi();
	initMenuConnect();
    D("初始化右键菜单完毕。");
}

Menu::~Menu()
{
    D("右键菜单删除中。");
    if (noSleepAct->isChecked())
    {
        A(MouseMoveTimer->stop();)
            A(delete MouseMoveTimer;)
    }
	wallpaper->wait();
	A(delete quitAct;)
    A(delete shutdownAct;)
    A(delete restartAct;)
    A(delete openFolderAct;)
    A(delete noSleepAct;)
    A(delete settingDialogAct;)
    A(delete nextPaperAct;)
    A(delete translateAct;)
    A(delete autoStartAct;)
    A(delete wallpaper;)
    D("菜单删除完毕");
}

void Menu::initUi()
{
	setWindowFlag(Qt::FramelessWindowHint);                       //没有任务栏图标
	setAttribute(Qt::WA_TranslucentBackground);                   //背景透明
#if defined(Q_OS_WIN32)
	QFile qss(":/qss/qss/menu_style_win.qss");
#elif defined(Q_OS_LINUX)
	QFile qss(":/qss/qss/menu_style_linux.qss");
#endif
	qss.open(QFile::ReadOnly);
	setStyleSheet(qss.readAll());
    qss.close();

	int font_use = QFontDatabase::addApplicationFont(":/fonts/fonts/smallkaiti.ttf");
	QStringList fontFamilies = QFontDatabase::applicationFontFamilies(font_use);
	QFont font;
	font.setFamily(fontFamilies.at(0));
#if defined(Q_OS_WIN32)
	font.setPointSize(10);
#elif defined(Q_OS_LINUX)
	font.setPointSize(9);
#endif
	setFont(font);
	setMaximumSize(MENU_WIDTH, MENU_HEIGHT);
	setMinimumSize(MENU_WIDTH, MENU_HEIGHT);
}

void Menu::initActions()
{
	wallpaper = new MenuWallpaper;                                //新建壁纸处理类

#if defined(Q_OS_WIN32)
	QString x = "    ";                                             //补充空格使右键菜单首字对齐
#elif defined(Q_OS_LINUX)
	QString x = "";
#endif
	autoStartAct = new QAction;
    autoStartAct->setText(x + "开机启动");
	autoStartAct->setCheckable(true);
	addAction(autoStartAct);

	translateAct = new QAction;
    translateAct->setText(x + "划词翻译");
	translateAct->setCheckable(true);
	addAction(translateAct);

	nextPaperAct = new QAction;
	nextPaperAct->setText(x + "换一张图");
	addAction(nextPaperAct);

	settingDialogAct = new QAction;
	settingDialogAct->setText(x + "壁纸设置");
	addAction(settingDialogAct);

	noSleepAct = new QAction;
    noSleepAct->setText(x + "防止息屏");
	noSleepAct->setCheckable(true);
	addAction(noSleepAct);

	openFolderAct = new QAction;
	openFolderAct->setText(x + "打开目录");
	addAction(openFolderAct);

	restartAct = new QAction;
	restartAct->setText(x + "快捷重启");
	addAction(restartAct);

	shutdownAct = new QAction;
	shutdownAct->setText(x + "快速关机");
	addAction(shutdownAct);

	quitAct = new QAction;
	quitAct->setText(x + "本次退出");
	addAction(quitAct);
}

void Menu::new_show(int x, int y)
{
	translateAct->setChecked(VarBox.ENABLE_TRANSLATER);
	setChecks();                               //设置是否选中“开机自启”
	moveMenuPos(x, y);
	show();
}

void Menu::moveMenuPos(int x, int y)           //自动把右键菜单移动到合适位置
{
	int px, py;
	if (x + MENU_WIDTH < VarBox.SCREEN_WIDTH)   //菜单右边界不超出屏幕时
		px = x;
	else
		px = VarBox.SCREEN_WIDTH - MENU_WIDTH;  //右边界和屏幕对齐
	if (y + MENU_HEIGHT < VarBox.SCREEN_HEIGHT) //菜单底部不超出屏幕底部时
		py = y;
	else
		py = y - MENU_HEIGHT;                  //菜单底部和鼠标对齐
	move(px, py);                              //移动右键菜单到 (px, py)
}

void Menu::initMenuConnect() const
{
	connect(wallpaper, SIGNAL(msgBox(QString)), form, SLOT(msgBox(QString)));
	connect(settingDialogAct, SIGNAL(triggered()), form->dialog, SLOT(showSelf()));   //打开壁纸设置界面
	connect(translateAct, SIGNAL(triggered(bool)), form, SLOT(enableTrans(bool)));    //是否启用翻译功能
	translateAct->setChecked(VarBox.ENABLE_TRANSLATER);                                 //设置是否选中“划词翻译”
	connect(nextPaperAct, &QAction::triggered, this,
		[=]() {
			if (wallpaper->isRunning())
				QMessageBox::information(nullptr, "提示", "频繁点击是没有效的哦！", QMessageBox::Ok, QMessageBox::Ok);
			else
				wallpaper->start();
		});
	connect(autoStartAct, SIGNAL(triggered(bool)), this, SLOT(setAutoStart(bool)));   //设置受否开机自启
	connect(noSleepAct, SIGNAL(triggered(bool)), this, SLOT(auto_move_cursor(bool))); //是否自动移动鼠标防止息屏
	connect(openFolderAct, SIGNAL(triggered()), this, SLOT(OpenFolder()));            //打开exe所在文件夹
	connect(restartAct, SIGNAL(triggered()), this, SLOT(RestartComputer()));          //重启电脑
	connect(shutdownAct, SIGNAL(triggered()), this, SLOT(ShutdownComputer()));        //关闭电脑
	connect(quitAct, &QAction::triggered, this, []() {
		VarBox.RUN_APP = false;                                                       //以便其它线程知晓，停止正在进行的工作
		VarBox.RUN_NET_CHECK = false;
		qApp->quit();                                                                 //退出程序
		});
}

void Menu::auto_move_cursor(bool checked)
{
	if (checked)                                                                      //启用自动移动鼠标
	{
		MouseMoveTimer = new QTimer;
		MouseMoveTimer->setInterval(45000);
		connect(MouseMoveTimer, SIGNAL(timeout()), this, SLOT(auto_move_cursor()));
		MouseMoveTimer->start();
	}
	else                                                                              //禁用自动移动鼠标
	{
		MouseMoveTimer->stop();
		disconnect(MouseMoveTimer, SIGNAL(timeout()), this, SLOT(auto_move_cursor()));
		delete MouseMoveTimer;
	}
}

#if defined(Q_OS_WIN32)
void Menu::auto_move_cursor() const                                                   //Windows下使用api函数移动鼠标
{
	int X = 1;
	int x = QCursor::pos().x();
	if (x == VarBox.SCREEN_WIDTH) X = -1;
	mouse_event(MOUSEEVENTF_MOVE, X, 0, 0, 0);                                        //移动一个像素
	mouse_event(MOUSEEVENTF_MOVE, -X, 0, 0, 0);                                       //移回原来的位置
}
#elif defined(Q_OS_LINUX)
void Menu::auto_move_cursor() const                                                   //Linux下使用Qt移动鼠标
{
	int temp_x;
	int x = QCursor::pos().x(), y = QCursor::pos().y();
	if (x == Form::SCREEN_WIDTH)
		temp_x = x - 1;
	else
		temp_x = x + 1;
	QCursor::setPos(temp_x, y);                                                       //移动一个像素
	QCursor::setPos(x, y);                                                            //移回原来位置
}
#endif


void Menu::OpenFolder() const
{
	QStringList argument;
#if defined(Q_OS_WIN32)                                                              //Windows下用 explorer xxx
	argument << VarBox.PATH_TO_OPEN.replace(QRegExp("/"), "\\");
	FuncBox::runCommand("explorer", argument);
#elif defined(Q_OS_LINUX)                                                            //Linux下用 xdg-open xxx
	argument << VarBox.PATH_TO_OPEN;
	FuncBox::runCommand("xdg-open", argument);
#endif
}

void Menu::ShutdownComputer() const
{
	QStringList argument;
#if defined(Q_OS_WIN32)                                                              //Windows下用 shutdown -s -f -t 0
	argument << "-s" << "-f" << "-t" << "0";
#elif defined(Q_OS_LINUX)
	argument << "-h" << "now";                                                       //Linux下用 shutdown -h now
#endif
	FuncBox::runCommand("shutdown", argument);
}

void Menu::RestartComputer() const
{
	QStringList argument;
#if defined(Q_OS_WIN32)                                                              //Windows下使用 shutdown -r -f -t 0
	argument << "-r" << "-f" << "-t" << "0";
	FuncBox::runCommand("shutdown", argument);
#elif defined(Q_OS_LINUX)                                                            //Linux下使用 reboot
	FuncBox::runCommand("reboot", argument);
#endif
}

void Menu::setChecks()
{
#if defined(Q_OS_WIN32)
	QSettings* reg = new QSettings(
		"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		QSettings::NativeFormat);
	autoStartAct->setChecked(!reg->value("SpeedBox").toString().compare(qApp->applicationFilePath().replace("/", "\\")));
	delete reg;

#elif defined(Q_OS_LINUX)                                                             //检查“autostart”目录下是否有.desktop文件
	if (QFile::exists(QDir::homePath() + "/.config/autostart/SpeedBox.desktop"))
		autoStartAct->setChecked(true);
	else
		autoStartAct->setChecked(false);
#endif
}

void Menu::setAutoStart(bool isSet) const        //将快捷方式复制到启动目录或者从启动目录删除快捷方式
{
#if defined(Q_OS_WIN32)
	QSettings* reg = new QSettings(
		"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		QSettings::NativeFormat);
	if (isSet)
		reg->setValue("SpeedBox", qApp->applicationFilePath().replace("/", "\\"));
	else
		reg->remove("SpeedBox");
	delete reg;
#elif defined(Q_OS_LINUX)
	QString linkpath = QDir::homePath() + "/.config/autostart/SpeedBox.desktop";
	QString nowpath = qApp->applicationDirPath() + "/Scripts/SpeedBox.desktop";
	if (isSet && (!QFile::exists(linkpath)))       //添加启动项
	{
		QFile file(nowpath);
		if (file.exists())
			D("找到启动文件。")
			if (QFile::copy(nowpath, linkpath))
			{
				D("成功复制到Start文件夹。")
					//赋予快捷方式可执行权限。
					QStringList lst;
				lst.append("+x");
				lst.append(linkpath);
				FuncBox::runCommand("chmod", lst, 0);
			}
			else
				D("复制启动文件失败。")
	}
#endif
}
