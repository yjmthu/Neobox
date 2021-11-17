#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include <QAction>
#include <QLabel>

#include "YString.h"
#include "YJson.h"
#include "funcbox.h"
#include "menu.h"
#include "form.h"
#include "wallpaper.h"

Menu::Menu(QWidget* parent) :
    QMenu(parent)
{
    initActions();
	initUi();
	initMenuConnect();
}

Menu::~Menu()
{
    qout << "析构menu开始";
    delete quitAct;
    delete shutdownAct;
    delete removePicAct;
    delete openFolderAct;
    delete noSleepAct;
    delete settingDialogAct;
    delete nextPaperAct;
    delete prevPaperAct;
    delete translateAct;
    qout << "析构menu结束";
}

void Menu::initUi()
{
    setWindowFlag(Qt::FramelessWindowHint);                       //没有任务栏图标
    setAttribute(Qt::WA_TranslucentBackground, true);                   //背景透明
    setAttribute(Qt::WA_DeleteOnClose, true);
    QFile qss(":/qss/menu_style.qss");                            //读取样式表
	qss.open(QFile::ReadOnly);
	setStyleSheet(qss.readAll());
    qss.close();

    QFont font;
    font.setFamily(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/smallkaiti.ttf")).at(0));
    font.setPointSize(10);
    setFont(font);

    setMaximumSize(MENU_WIDTH, MENU_HEIGHT);                      //限定大小
	setMinimumSize(MENU_WIDTH, MENU_HEIGHT);
}

void Menu::initActions()
{
    settingDialogAct = new QAction;
    settingDialogAct->setText("软件设置");
    addAction(settingDialogAct);

    translateAct = new QAction;
    translateAct->setText("划词翻译");
    translateAct->setCheckable(true);
    addAction(translateAct);

    prevPaperAct = new QAction;
    prevPaperAct->setText("上一张图");
    addAction(prevPaperAct);

    nextPaperAct = new QAction;
    nextPaperAct->setText("下一张图");
    addAction(nextPaperAct);

    removePicAct = new QAction;
    removePicAct->setText("不看此图");
    addAction(removePicAct);

    openFolderAct = new QAction;
    openFolderAct->setText("打开目录");
	addAction(openFolderAct);

    shutdownAct = new QAction;
    shutdownAct->setText("快速关机");
	addAction(shutdownAct);

    quitAct = new QAction;
    quitAct->setText("本次退出");
	addAction(quitAct);

    noSleepAct = new QAction;
    noSleepAct->setText("防止息屏");
    noSleepAct->setCheckable(true);
    addAction(noSleepAct);
}


void Menu::showEvent(QShowEvent *event)
{
    translateAct->setChecked(VarBox->EnableTranslater);
    event->accept();
}

void Menu::Show(int x, int y)           //自动把右键菜单移动到合适位置
{
    noSleepAct->setChecked(VarBox->form->MouseMoveTimer);
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
    connect(settingDialogAct, &QAction::triggered, [](){
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
    });                                                                                  //打开壁纸设置界面
    connect(translateAct, &QAction::triggered, VarBox->form, &Form::enableTranslater);    //是否启用翻译功能
    translateAct->setChecked(VarBox->EnableTranslater);                                 //设置是否选中“划词翻译”
    connect(nextPaperAct, &QAction::triggered, VarBox->wallpaper, &Wallpaper::next);
    connect(prevPaperAct, &QAction::triggered, VarBox->wallpaper, &Wallpaper::prev);   //设置受否开机自启
    connect(noSleepAct, &QAction::triggered, VarBox, [=](bool checked){
        if (checked)                                                                      //启用自动移动鼠标
        {
            VarBox->form->MouseMoveTimer = new QTimer;
            VarBox->form->MouseMoveTimer->setInterval(45000);
            connect(VarBox->form->MouseMoveTimer, &QTimer::timeout, [](){
                int X = 1;
                if (QCursor::pos().x() == VarBox->ScreenWidth) X = -1;
                mouse_event(MOUSEEVENTF_MOVE, X, 0, 0, 0);                                        //移动一个像素
                mouse_event(MOUSEEVENTF_MOVE, -X, 0, 0, 0);                                       //移回原来的位置
            });
            VarBox->form->MouseMoveTimer->start();
        }
        else                                                                              //禁用自动移动鼠标
        {
            VarBox->form->MouseMoveTimer->stop();
            delete VarBox->form->MouseMoveTimer;
            VarBox->form->MouseMoveTimer = nullptr;
        }
    });                                                                               //是否自动移动鼠标防止息屏
	connect(openFolderAct, SIGNAL(triggered()), this, SLOT(OpenFolder()));            //打开exe所在文件夹
    connect(removePicAct, &QAction::triggered, VarBox->wallpaper, &Wallpaper::dislike);          //重启电脑
	connect(shutdownAct, SIGNAL(triggered()), this, SLOT(ShutdownComputer()));        //关闭电脑
    connect(quitAct, &QAction::triggered, qApp, &QCoreApplication::quit);             // 退出程序
}


void Menu::OpenFolder() const
{
	QStringList argument;                                                           //Windows下用 explorer xxx
    argument << VarBox->PathToOpen;
    //qout << "打开目录" << VarBox->PathToOpen;
    VarBox->runCmd("explorer.exe", argument);
}

void Menu::ShutdownComputer() const
{
    HANDLE hToken; TOKEN_PRIVILEGES tkp;
    typedef BOOL (*pfnOpenProcessToken)(HANDLE  ProcessHandle,DWORD DesiredAccess,PHANDLE TokenHandle);
    typedef BOOL (*pfnLookupPrivilegeValue)(LPCWSTR lpSystemName,LPCWSTR lpName,PLUID  lpLuid);
    typedef BOOL (*pfnAdjustTokenPrivileges)(HANDLE TokenHandle,BOOL DisableAllPrivileges,PTOKEN_PRIVILEGES NewState, DWORD BufferLength,PTOKEN_PRIVILEGES PreviousState,PDWORD ReturnLength);
    typedef BOOL (*pfnInitiateSystemShutdownEx)(LPSTR lpMachineName,LPSTR lpMessage,DWORD dwTimeout,BOOL  bForceAppsClosed,BOOL  bRebootAfterShutdown,DWORD dwReason);
    HMODULE hAdvapi32 = LoadLibraryA("Advapi32.dll");
    pfnOpenProcessToken pOpenProcessToken = NULL;
    pfnLookupPrivilegeValue pLookupPrivilegeValue = NULL;
    pfnAdjustTokenPrivileges pAdjustTokenPrivileges = NULL;
    pfnInitiateSystemShutdownEx pInitiateSystemShutdownEx = NULL;
    if (hAdvapi32)
    {
        pOpenProcessToken = (pfnOpenProcessToken)GetProcAddress(hAdvapi32, "OpenProcessToken");
        pLookupPrivilegeValue = (pfnLookupPrivilegeValue)GetProcAddress(hAdvapi32, "LookupPrivilegeValueW");
        pAdjustTokenPrivileges = (pfnAdjustTokenPrivileges)GetProcAddress(hAdvapi32, "AdjustTokenPrivileges");
        pInitiateSystemShutdownEx = (pfnInitiateSystemShutdownEx)GetProcAddress(hAdvapi32, "InitiateSystemShutdownExW");
    }
    if (!pOpenProcessToken || !pLookupPrivilegeValue || !pAdjustTokenPrivileges || !pInitiateSystemShutdownEx || !pOpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        VarBox->MSG("获取模块失败", "警告", QMessageBox::Ok);
        return;
    }
    pLookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    pAdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL);
    if (GetLastError() != ERROR_SUCCESS)
    {
        VarBox->MSG("关机失败", "警告", QMessageBox::Ok);
        return;
    }
    pInitiateSystemShutdownEx(NULL, NULL, 0, TRUE, FALSE, NULL);
    FreeLibrary(hAdvapi32);
}
