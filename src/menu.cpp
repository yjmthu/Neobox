#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QAction>
#include <QLabel>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QApplication>
#include "dialog.h"
#include "YString.h"
#include "YJson.h"
#include "funcbox.h"
#include "menu.h"
#include "form.h"
#include "wallpaper.h"
#include "calculator.h"

constexpr int MENU_WIDTH = 90;
constexpr int MENU_HEIGHT = 330;

bool Menu::keepScreenOn = false;

Menu::Menu(QWidget* parent) :
    QMenu(parent),
    actions(new QAction[11])
{
    initActions();
	initUi();
	initMenuConnect();
}

Menu::~Menu()
{
    qout << "析构menu开始";
    delete [] actions;
    qout << "析构menu结束";
}

void Menu::initUi()
{
    setWindowFlag(Qt::FramelessWindowHint);                       //没有任务栏图标
    setAttribute(Qt::WA_TranslucentBackground, true);             //背景透明
    setAttribute(Qt::WA_DeleteOnClose, true);

    QFile qss(QStringLiteral(":/qss/menu_style.qss"));                            //读取样式表
	qss.open(QFile::ReadOnly);
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral("://fonts/FangZhengKaiTiJianTi-1.ttf"));
    QStringList fontIDs = QFontDatabase::applicationFontFamilies(fontId);
    if (! fontIDs.isEmpty()) {
        setStyleSheet(QString(qss.readAll()).arg(fontIDs.first()));
    } else {
        qDebug()<<"Failed to load font.";
    }
    qss.close();

    setMaximumSize(MENU_WIDTH, MENU_HEIGHT);                      //限定大小
	setMinimumSize(MENU_WIDTH, MENU_HEIGHT);
}

void Menu::initActions()
{
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    constexpr char lst[11][17] = {
        "    软件设置", "    科学计算", "划词翻译", "    上一张图", "    下一张图",
        "    不看此图", "    打开目录", "    快速关机", "    快捷重启", "    本次退出",
        "防止息屏"
    };
#else
    constexpr char lst[11][13] = {
        "软件设置", "科学计算", "划词翻译", "上一张图", "下一张图",
        "不看此图", "打开目录", "快速关机", "快捷重启", "本次退出",
        "防止息屏"
    };
#endif
    for (int i = 0; i < 11; ++i)
    {
        actions[i].setText(lst[i]);
        addAction(actions+i);
    }
    actions[2].setCheckable(true);
    actions[10].setCheckable(true);
}

void Menu::showEvent(QShowEvent *event)
{
    actions[2].setChecked(VarBox->EnableTranslater);
    event->accept();
}

void Menu::Show(int x, int y)                   //自动把右键菜单移动到合适位置
{
    actions[10].setChecked(keepScreenOn);
	int px, py;
    if (x + MENU_WIDTH < VarBox->ScreenWidth)   //菜单右边界不超出屏幕时
		px = x;
	else
        px = VarBox->ScreenWidth - MENU_WIDTH;  //右边界和屏幕对齐
    if (y + MENU_HEIGHT < VarBox->ScreenHeight) //菜单底部不超出屏幕底部时
		py = y;
	else
		py = y - MENU_HEIGHT;                  //菜单底部和鼠标对齐
    move(px, py);                              //移动右键菜单到 (px, py)
    show();
}

void Menu::initMenuConnect()
{
    connect(actions, &QAction::triggered, VarBox, [](){
        if (VarBox->dialog)
        {
            if (VarBox->dialog->isVisible())
                VarBox->dialog->setWindowState(Qt::WindowActive | Qt::WindowNoState);    // 让窗口从最小化恢复正常并激活窗口
                //d->activateWindow();
                //d->raise();
            else
                VarBox->dialog->show();
        }
        else
        {
            *const_cast<Dialog**>(&(VarBox->dialog)) = new Dialog;
            VarBox->dialog->show();
        }
    });                                         //打开壁纸设置界面
    connect(actions+1, &QAction::triggered, VarBox, [](){
        Calculator lator;
        lator.exec();
    });
    connect(actions+2, &QAction::triggered, VarBox->form, &Form::enableTranslater);    //是否启用翻译功能
    actions[2].setChecked(VarBox->EnableTranslater);                                   //设置是否选中“划词翻译”
    connect(actions+4, &QAction::triggered, VarBox->wallpaper, &Wallpaper::next);
    connect(actions+3, &QAction::triggered, VarBox->wallpaper, &Wallpaper::prev);      //设置受否开机自启                                                                              //是否自动移动鼠标防止息屏
    connect(actions+5, &QAction::triggered, VarBox->wallpaper, &Wallpaper::dislike);
    connect(actions+6, &QAction::triggered, VarBox, [](){
        qout << VarBox->PathToOpen;
        VarBox->openDirectory(VarBox->PathToOpen);
    });                 //打开exe所在文件夹
#ifdef Q_OS_WIN32
    connect(actions+7, &QAction::triggered, VarBox,
            std::bind(
                (QByteArray (*)(const QString &, const QStringList&, short))
                VARBOX::runCmd, QStringLiteral("shutdown"), QStringList({"-s", "-t", "0"}), 0)
            );            //关闭电脑
    connect(actions+8, &QAction::triggered, VarBox,
            std::bind(
                (QByteArray (*)(const QString &, const QStringList&, short))
                VARBOX::runCmd, QStringLiteral("shutdown"), QStringList({"-r", "-t", "0"}), 0)
            );
#elif defined Q_OS_LINUX
    connect(actions+7, &QAction::triggered, VarBox, [](){system("shutdown -h now");});            //关闭电脑
    connect(actions+8, &QAction::triggered, VarBox, [](){system("reboot");});
#endif
    connect(actions+9, &QAction::triggered, qApp, &QCoreApplication::quit);            // 退出程序
    connect(actions+10, &QAction::triggered, VarBox, [=](bool checked){
        if ((keepScreenOn = checked))                                                                   //启用自动移动鼠标
        {
            SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
        } else {
            SetThreadExecutionState(ES_CONTINUOUS);
        }
    });
}
