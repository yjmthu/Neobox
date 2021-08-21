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
#include "menuwallpaper.h"

template<typename  T>
T get_file_name(T file_path)
{
    T ptr = file_path;
    while (*++ptr);
    while(*--ptr != '\\');
    return StrJoin(++ptr);
}

void check_is_wallhaven(const wchar_t* pic, char* id)
{
    qout << 99;
    if (wcslen(pic) != 20)
        return ;
    if (wcsncmp(pic, L"wallhaven-", 10) && StrContainRange(pic+10, 6, L"a-z", L"0-9") &&
            (wcscmp(pic+16, L".png") || wcscmp(pic+16, L".jpg")))
    {
        for (int i=0; i <= 5; ++i)
        {
            id[i] = static_cast<char>(pic[10+i]);
        }
    }
}

Menu::Menu() :
    QMenu(nullptr)
{
    initActions();
	initUi();
	initMenuConnect();
}

Menu::~Menu()
{
    qout << "析构menu开始";
    if (noSleepAct->isChecked())
    {
        MouseMoveTimer->stop();
        delete MouseMoveTimer;
    }
    delete quitAct;
    delete shutdownAct;
    delete removePicAct;
    delete openFolderAct;
    delete noSleepAct;
    delete settingDialogAct;
    delete nextPaperAct;
    delete prevPaperAct;
    delete translateAct;
    delete wallpaper;
    qout << "析构menu结束";
}

void Menu::initUi()
{
    setWindowFlag(Qt::FramelessWindowHint);                       //没有任务栏图标
	setAttribute(Qt::WA_TranslucentBackground);                   //背景透明
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
    wallpaper = new MenuWallpaper;                                //新建壁纸处理类

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

    noSleepAct = new QAction;
    noSleepAct->setText("防止息屏");
    noSleepAct->setCheckable(true);
    addAction(noSleepAct);

    openFolderAct = new QAction;
    openFolderAct->setText("打开目录");
	addAction(openFolderAct);

    shutdownAct = new QAction;
    shutdownAct->setText("快速关机");
	addAction(shutdownAct);

    quitAct = new QAction;
    quitAct->setText("本次退出");
	addAction(quitAct);
}


void Menu::showEvent(QShowEvent *event)
{
    translateAct->setChecked(VarBox->EnableTranslater);
    event->accept();
}

void Menu::Show(int x, int y)           //自动把右键菜单移动到合适位置
{
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
    connect(wallpaper, &MenuWallpaper::msgBox, VarBox->form, &Form::msgBox);
    connect(settingDialogAct, &QAction::triggered, [](){
        if (VarBox->form->dialog->isVisible())
        {
            qout << "as 1";
            VarBox->form->dialog->setWindowState(Qt::WindowActive | Qt::WindowNoState);    // 让窗口从最小化恢复正常并激活窗口
            //d->activateWindow();
            //d->raise();
        }
        else
        {
            qout << "as 2";
            VarBox->form->dialog->show();
        }
    });   //打开壁纸设置界面
    connect(translateAct, &QAction::triggered, VarBox->form, &Form::enableTranslater);    //是否启用翻译功能
    translateAct->setChecked(VarBox->EnableTranslater);                                 //设置是否选中“划词翻译”
    connect(nextPaperAct, &QAction::triggered,
        [=]() {
            if (wallpaper->isActive())
                QMessageBox::information(VarBox->form->dialog, "提示", "频繁点击是没有效的哦！", QMessageBox::Ok, QMessageBox::Ok);
            else if (Wallpaper::canCreat())
                wallpaper->start();
        });
    connect(prevPaperAct, &QAction::triggered, [](){
        //QString str =
        while (true)
        {
            if (VarBox->CurPic == VarBox->PicHistory.begin())
            {
                QMessageBox::information(VarBox->form->dialog, "提示", "无法找到更早的壁纸历史记录！", QMessageBox::Ok, QMessageBox::Ok);
                return ;
            }
            if ((--VarBox->CurPic)->first)
            {
                if (VARBOX::PathFileExists(static_cast<wchar_t*>(VarBox->CurPic->second)))
                {
                    SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                    return;
                }
                delete [] static_cast<wchar_t*>(VarBox->CurPic->second);
            }
            else
            {
                if (VARBOX::PathFileExists(static_cast<char*>(VarBox->CurPic->second)))
                {
                    SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                    return;
                }
                delete [] static_cast<char*>(VarBox->CurPic->second);
            }
            VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
            if (VarBox->CurPic != VarBox->PicHistory.begin())
                --VarBox->CurPic;
        }
    });   //设置受否开机自启
    connect(noSleepAct, &QAction::triggered, [=](bool checked){
        if (checked)                                                                      //启用自动移动鼠标
        {
            MouseMoveTimer = new QTimer;
            MouseMoveTimer->setInterval(45000);
            connect(MouseMoveTimer, &QTimer::timeout, [](){
                int X = 1;
                if (QCursor::pos().x() == VarBox->ScreenWidth) X = -1;
                mouse_event(MOUSEEVENTF_MOVE, X, 0, 0, 0);                                        //移动一个像素
                mouse_event(MOUSEEVENTF_MOVE, -X, 0, 0, 0);                                       //移回原来的位置
            });
            MouseMoveTimer->start();
        }
        else                                                                              //禁用自动移动鼠标
        {
            MouseMoveTimer->stop();
            delete MouseMoveTimer;
        }
    }); //是否自动移动鼠标防止息屏
	connect(openFolderAct, SIGNAL(triggered()), this, SLOT(OpenFolder()));            //打开exe所在文件夹
    connect(removePicAct, &QAction::triggered, [this](){
        if (!Wallpaper::canCreat())
        {
            QMessageBox::information(VarBox->form->dialog,"提示", "后台正忙，请稍后！");
            return ;
        }
        qout << "不喜欢该壁纸。";
        if (VarBox->PaperType == PAPER_TYPE::Bing && !VarBox->AutoRotationBingPicture)
        {
            QMessageBox::information(VarBox->form->dialog, "提示", "壁纸切换列表中没有替代方案，如果不喜欢此张壁纸，\n请切换壁纸类型或者启用必应壁纸轮换。");
            return;
        }
        if (VarBox->CurPic->first)
        {
            const wchar_t* pic_path = static_cast<const wchar_t*>(VarBox->CurPic->second);
            //qout << "图片路径：" << QString::fromWCharArray(pic_path);
            const wchar_t* pic_name = get_file_name(pic_path);
            //qout << "图片名称：" << QString::fromWCharArray(pic_name);
            char id[7] = {0};
            check_is_wallhaven(pic_name, id);
            if (*id)
            {
                QString str = VarBox->get_dat_path() + "\\Blacklist.json";
                YJsonItem *blackList = nullptr;
                if (QFile::exists(str))
                {
                    blackList = YJsonItem::newFromFile(str.toStdWString());
                    if (blackList->getType() != YJSON_TYPE::YJSON_ARRAY)
                    {
                        delete blackList;
                        blackList = YJsonItem::newArray();
                    }
                }
                else
                    blackList = YJsonItem::newArray();
                blackList->appendItem(id);
                blackList->toFile(str.toStdWString(), YJSON_ENCODE::UTF8);
                delete  blackList;
            }
            DeleteFileW(pic_path);
            delete [] pic_path;
        }
        else
        {
            const char* pic_path = static_cast<const char*>(VarBox->CurPic->second);
            DeleteFileA(pic_path);
            delete [] pic_path;
        }
        VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
        while (true)
        {
            if (VarBox->CurPic != VarBox->PicHistory.begin())
            {
                --VarBox->CurPic;
                wallpaper->start();
                break;
            }
            if (VarBox->CurPic == VarBox->PicHistory.end())
            {
                wallpaper->start();
                break;
            }

            if (VarBox->CurPic->first)
            {
                if (VARBOX::PathFileExists(static_cast<wchar_t*>(VarBox->CurPic->second)))
                {
                    SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                    break;
                }
                else
                {
                    delete [] static_cast<wchar_t*>(VarBox->CurPic->second);
                    VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
                }
            }
            else
            {
                if (VARBOX::PathFileExists(static_cast<char*>(VarBox->CurPic->second)))
                {
                    SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                    break;
                }
                else
                {
                    delete [] static_cast<char*>(VarBox->CurPic->second);
                    VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
                }
            }
        }
    });          //重启电脑
	connect(shutdownAct, SIGNAL(triggered()), this, SLOT(ShutdownComputer()));        //关闭电脑
    connect(quitAct, &QAction::triggered, []() {
        VarBox->RunApp = false;                                                       //以便其它线程知晓，停止正在进行的工作
		qApp->quit();                                                                 //退出程序
		});
}


void Menu::OpenFolder() const
{
	QStringList argument;                                                           //Windows下用 explorer xxx
    argument << VarBox->PathToOpen;
    //qout << "打开目录" << VarBox->PathToOpen;
    VARBOX::runCommand("explorer.exe", argument);
}

void Menu::ShutdownComputer() const
{
    HANDLE hToken; TOKEN_PRIVILEGES tkp;
    typedef BOOL (*pfnOpenProcessToken)(HANDLE  ProcessHandle,DWORD   DesiredAccess,PHANDLE TokenHandle);
    typedef BOOL (*pfnLookupPrivilegeValue)(LPCSTR lpSystemName,LPCSTR lpName,PLUID  lpLuid);
    typedef BOOL (*pfnAdjustTokenPrivileges)(HANDLE TokenHandle,BOOL DisableAllPrivileges,PTOKEN_PRIVILEGES NewState, DWORD BufferLength,PTOKEN_PRIVILEGES PreviousState,PDWORD ReturnLength);
    typedef BOOL (*pfnInitiateSystemShutdownEx)(LPSTR lpMachineName,LPSTR lpMessage,DWORD dwTimeout,BOOL  bForceAppsClosed,BOOL  bRebootAfterShutdown,DWORD dwReason);
    HMODULE hAdvapi32 = LoadLibrary("Advapi32.dll");
    pfnOpenProcessToken pOpenProcessToken = NULL;
    pfnLookupPrivilegeValue pLookupPrivilegeValue = NULL;
    pfnAdjustTokenPrivileges pAdjustTokenPrivileges = NULL;
    pfnInitiateSystemShutdownEx pInitiateSystemShutdownEx = NULL;
    if (hAdvapi32)
    {
        pOpenProcessToken = (pfnOpenProcessToken)GetProcAddress(hAdvapi32, "OpenProcessToken");
        pLookupPrivilegeValue = (pfnLookupPrivilegeValue)GetProcAddress(hAdvapi32, "LookupPrivilegeValueA");
        pAdjustTokenPrivileges = (pfnAdjustTokenPrivileges)GetProcAddress(hAdvapi32, "AdjustTokenPrivileges");
        pInitiateSystemShutdownEx = (pfnInitiateSystemShutdownEx)GetProcAddress(hAdvapi32, "InitiateSystemShutdownExA");
    }
    if (!pOpenProcessToken || !pLookupPrivilegeValue || !pAdjustTokenPrivileges || !pInitiateSystemShutdownEx || !pOpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        QMessageBox::warning(VarBox->form->dialog, "警告", "获取模块失败", QMessageBox::Ok);
        return;
    }
    pLookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    pAdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL);
    if (GetLastError() != ERROR_SUCCESS)
    {
        QMessageBox::warning(VarBox->form->dialog, "警告", "关机失败", QMessageBox::Ok);
        return;
    }
    pInitiateSystemShutdownEx(NULL, NULL, 0, TRUE, FALSE, NULL);
    FreeLibrary(hAdvapi32);
}
