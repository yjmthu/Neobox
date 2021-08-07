#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include <QAction>
#include <QLabel>

#include "funcbox.h"
#include "menu.h"
#include "form.h"
#include "menuwallpaper.h"

Menu::Menu() :
    QMenu(nullptr)
{
    initActions();
	initUi();
	initMenuConnect();
}

Menu::~Menu()
{
    if (noSleepAct->isChecked())
    {
        MouseMoveTimer->stop();
        delete MouseMoveTimer;
    }
    delete quitAct;
    delete shutdownAct;
    delete restartAct;
    delete openFolderAct;
    delete noSleepAct;
    delete settingDialogAct;
    delete nextPaperAct;
    delete translateAct;
    delete autoStartAct;
    delete wallpaper;
}

void Menu::initUi()
{
    setWindowFlag(Qt::FramelessWindowHint);                       //没有任务栏图标
	setAttribute(Qt::WA_TranslucentBackground);                   //背景透明
    QFile qss(":/qss/menu_style.qss");
	qss.open(QFile::ReadOnly);
	setStyleSheet(qss.readAll());
    qss.close();

    int font_use = QFontDatabase::addApplicationFont(":/fonts/smallkaiti.ttf");
	QStringList fontFamilies = QFontDatabase::applicationFontFamilies(font_use);
	QFont font;
	font.setFamily(fontFamilies.at(0));
	font.setPointSize(10);
	setFont(font);
	setMaximumSize(MENU_WIDTH, MENU_HEIGHT);
	setMinimumSize(MENU_WIDTH, MENU_HEIGHT);
}

void Menu::initActions()
{
    wallpaper = new MenuWallpaper;                                //新建壁纸处理类

    autoStartAct = new QAction;
    autoStartAct->setText("开机启动");
	autoStartAct->setCheckable(true);
	addAction(autoStartAct);

    translateAct = new QAction;
    translateAct->setText("划词翻译");
	translateAct->setCheckable(true);
	addAction(translateAct);

    nextPaperAct = new QAction;
    nextPaperAct->setText("换一张图");
	addAction(nextPaperAct);

    settingDialogAct = new QAction;
    settingDialogAct->setText("软件设置");
	addAction(settingDialogAct);

    noSleepAct = new QAction;
    noSleepAct->setText("防止息屏");
	noSleepAct->setCheckable(true);
	addAction(noSleepAct);

    openFolderAct = new QAction;
    openFolderAct->setText("打开目录");
	addAction(openFolderAct);

    restartAct = new QAction;
    restartAct->setText("快捷重启");
	addAction(restartAct);

    shutdownAct = new QAction;
    shutdownAct->setText("快速关机");
	addAction(shutdownAct);

    quitAct = new QAction;
    quitAct->setText("本次退出");
	addAction(quitAct);
}


void Menu::showEvent(QShowEvent *event)
{
    translateAct->setChecked(VarBox.EnableTranslater);
    setChecks();                               //设置是否选中“开机自启”
    event->accept();
}

void Menu::Show(int x, int y)           //自动把右键菜单移动到合适位置
{
	int px, py;
    if (x + MENU_WIDTH < VarBox.ScreenWidth)   //菜单右边界不超出屏幕时
		px = x;
	else
        px = VarBox.ScreenWidth - MENU_WIDTH;  //右边界和屏幕对齐
    if (y + MENU_HEIGHT < VarBox.ScreenHeight) //菜单底部不超出屏幕底部时
		py = y;
	else
		py = y - MENU_HEIGHT;                  //菜单底部和鼠标对齐
	move(px, py);                              //移动右键菜单到 (px, py)
    show();
}

void Menu::initMenuConnect() const
{
    connect(wallpaper, &MenuWallpaper::msgBox, (Form*)VarBox.form, &Form::msgBox);
    connect(settingDialogAct, &QAction::triggered, this, [](){
        Dialog *d = ((Form*)VarBox.form)->dialog;
        if (d->isVisible())
        {
            qout << "as 1";
            d->setWindowState(Qt::WindowActive | Qt::WindowNoState);
            //d->activateWindow();
            //d->raise();
        }
        else
        {
            qout << "as 2";
            d->show();
        }
    });   //打开壁纸设置界面
    connect(translateAct, SIGNAL(triggered(bool)), (Form*)VarBox.form, SLOT(enableTrans(bool)));    //是否启用翻译功能
    translateAct->setChecked(VarBox.EnableTranslater);                                 //设置是否选中“划词翻译”
    connect(nextPaperAct, &QAction::triggered, this,
        [=]() {
            if (wallpaper->isActive())
                QMessageBox::information(nullptr, "提示", "频繁点击是没有效的哦！", QMessageBox::Ok, QMessageBox::Ok);
            else if (Wallpaper::canCreat())
                wallpaper->start();
        });
	connect(autoStartAct, SIGNAL(triggered(bool)), this, SLOT(setAutoStart(bool)));   //设置受否开机自启
	connect(noSleepAct, SIGNAL(triggered(bool)), this, SLOT(auto_move_cursor(bool))); //是否自动移动鼠标防止息屏
	connect(openFolderAct, SIGNAL(triggered()), this, SLOT(OpenFolder()));            //打开exe所在文件夹
	connect(restartAct, SIGNAL(triggered()), this, SLOT(RestartComputer()));          //重启电脑
	connect(shutdownAct, SIGNAL(triggered()), this, SLOT(ShutdownComputer()));        //关闭电脑
    connect(quitAct, &QAction::triggered, this, []() {
        VarBox.RunApp = false;                                                       //以便其它线程知晓，停止正在进行的工作
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

void Menu::auto_move_cursor() const                                                   //Windows下使用api函数移动鼠标
{
	int X = 1;
	int x = QCursor::pos().x();
    if (x == VarBox.ScreenWidth) X = -1;
	mouse_event(MOUSEEVENTF_MOVE, X, 0, 0, 0);                                        //移动一个像素
	mouse_event(MOUSEEVENTF_MOVE, -X, 0, 0, 0);                                       //移回原来的位置
}


void Menu::OpenFolder() const
{
	QStringList argument;                                                           //Windows下用 explorer xxx
    argument << VarBox.PathToOpen;
    qout << "打开目录" << VarBox.PathToOpen;
    FuncBox::runCommand("explorer.exe", argument);
}

void Menu::ShutdownComputer() const
{
	QStringList argument;                                                          //Windows下用 shutdown -s -f -t 0
	argument << "-s" << "-f" << "-t" << "0";
	FuncBox::runCommand("shutdown", argument);
}

void Menu::RestartComputer() const
{
	QStringList argument;                                                           //Windows下使用 shutdown -r -f -t 0
	argument << "-r" << "-f" << "-t" << "0";
	FuncBox::runCommand("shutdown", argument);
}

void Menu::setChecks()
{
	QSettings* reg = new QSettings(
		"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		QSettings::NativeFormat);
	autoStartAct->setChecked(!reg->value("SpeedBox").toString().compare(qApp->applicationFilePath().replace("/", "\\")));
	delete reg;
}

void Menu::setAutoStart(bool isSet) const        //将快捷方式复制到启动目录或者从启动目录删除快捷方式
{
	QSettings* reg = new QSettings(
		"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		QSettings::NativeFormat);
	if (isSet)
		reg->setValue("SpeedBox", qApp->applicationFilePath().replace("/", "\\"));
	else
		reg->remove("SpeedBox");
	delete reg;
}
