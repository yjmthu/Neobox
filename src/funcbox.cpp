#include <fstream>
#include <oaidl.h>
#include <WinInet.h>
#include <QApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QScreen>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextCodec>

#include "funcbox.h"
#include "YString.h"
#include "YJson.h"
#include "form.h"
#include "desktopmask.h"
#include "wallpaper.h"
#include "wallpaper.h"


wchar_t* GetCorrectUnicode(const QByteArray &ba)
{
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    codec->toUnicode(ba.constData(), ba.size(), &state);
    QString str = (state.invalidChars > 0) ? QTextCodec::codecForName("GBK")->toUnicode(ba): ba;
    qout << "转换后的字符串: " << str;
    auto s = reinterpret_cast<const wchar_t*>(str.utf16());
    auto l = wcslen(s)+1;
    auto t = new wchar_t[l];
    std::copy(s, s+l+1, t);
    return t;
}

VARBOX* VarBox = nullptr;
HANDLE VARBOX::HMutex = NULL;

VARBOX::VARBOX(int w, int h):
    QObject(nullptr),
    ScreenWidth(w), ScreenHeight(h), SysScreenWidth(GetSystemMetrics(SM_CXSCREEN)), SysScreenHeight(GetSystemMetrics(SM_CYSCREEN)),
    hOleacc(LoadLibraryA("oleacc.dll")), hDwmapi(LoadLibraryA("dwmapi.dll")), hWininet(LoadLibraryA("wininet.dll"))
{
    qout << "VarBox构造函数开始。";
    VarBox = this;
    QString data_path = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +  "/AppData/Local/SpeedBox");
    QDir().mkdir(data_path);
    QDir::setCurrent(data_path);
    loadFunctions();
    initFile();
    initChildren();
    initBehaviors();
    qout << "VarBox构造函数结束。";
}

VARBOX::~VARBOX()
{
    qout << "结构体析构中~";
    delete form;
    qout << "析构任务栏类";
    qout << "析构壁纸类";
    delete wallpaper;
    qout << "析构设置对话框";
    delete dialog;
    qout << "析构桌面图标控制类";
    delete ControlDesktopIcon;
    delete [] AppId;
    delete [] PassWord;
    if (hOleacc) FreeLibrary(hOleacc);
    if (hDwmapi) FreeLibrary(hDwmapi);
    qout << "结构体析构成功。";
}

void VARBOX::loadFunctions()
{
#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
    constexpr DWORD FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS = 0x00400000;
#endif
    if (hOleacc)
    {
        AccessibleObjectFromWindow = (pfnAccessibleObjectFromWindow)GetProcAddress(hOleacc, "AccessibleObjectFromWindow");
        AccessibleChildren = (pfnAccessibleChildren)GetProcAddress(hOleacc, "AccessibleChildren");
    }

    if (hDwmapi)
    {
        pDwmGetWindowAttribute = (pfnDwmGetWindowAttribute)GetProcAddress(hDwmapi, "DwmGetWindowAttribute");
    }

    if (hWininet)
    {
        static const auto pInternetGetConnectedState = (pfnInternetGetConnectedState)GetProcAddress(hWininet, "InternetGetConnectedState");
        InternetGetConnectedState = []()->bool{
            DWORD flag;
            return pInternetGetConnectedState(&flag, 0);
        };
    }


    if (!(AccessibleObjectFromWindow && AccessibleChildren && pDwmGetWindowAttribute))
    {
        qApp->exit(RETCODE_ERROR_EXIT);
    }
    OneDriveFile = [this](const wchar_t*file)->bool{
        if (FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS & GetFileAttributesW(file))
        {
            if (!InternetGetConnectedState()) return false;
            for (int i = 0; i < 30; i++)
            {
                HANDLE hFileRead = CreateFileW(reinterpret_cast<const wchar_t*>(file), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if(hFileRead==INVALID_HANDLE_VALUE)
                {
                    qout << "非法路径";
                    return false;
                }
                char lpFileDataBuffer[1] = {0};
                DWORD dwReadedSize = 0;
                if(ReadFile(hFileRead,lpFileDataBuffer,1,&dwReadedSize, NULL) && dwReadedSize)
                {
                    CloseHandle(hFileRead);
                    return true;
                }
                CloseHandle(hFileRead);
                Sleep(1000);
            }
        }
        else
        {
            return true;
        }
        return false;
    };
    PathFileExists = [](const wchar_t* pszPath)->bool{
        typedef BOOL(WINAPI* pfnPathFileExists)(LPWSTR);
        pfnPathFileExists pPathFileExists = NULL;
        BOOL ret = FALSE;
        HMODULE hUser = GetModuleHandleW(L"Shlwapi.dll");
        if (hUser)
            pPathFileExists = (pfnPathFileExists)GetProcAddress(hUser, "PathFileExistsW");
        if (pPathFileExists)
        {
            ret = pPathFileExists((wchar_t*)pszPath);
        }
        else
        {
        }
        return ret;
    };
    GetFileAttributes = [](const wchar_t* ph)->bool{
        return GetFileAttributesW(ph) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS;
    };
}

bool VARBOX::check_app_right()
{
    if (!QFile::exists("AppId.txt")) return false;
    QFile file("AppId.txt");
    if( file.open(QIODevice::ReadOnly))
    {
        qout << "成功打开密钥文件";
        auto size = file.size();
        if (size < 20)
        {
            file.close();
            return false;
        }
        unsigned char c[3]  = { 0 };
        if (!file.read((char*)c, sizeof(char)*3))
        {
            qout << "文件读取失败1";
            file.close();
            return false;
        }
        size -= sizeof(char) * 3;
        if (c[0] == 0xef && c[1] == 0xbb && c[2] == 0xbf)
        {
            char *str = new char[(size_t)size + 1]; str[size] = 0;
            if (!file.read(str, size))
            {
                qout << "文件读取失败2";
                file.close();
                delete[] str; return false;
            }
            else
            {
                char* ptr0 = str + 7; char* ptr = ptr0; while (*++ptr && *ptr != '\n');
                if (*ptr) {
                    *ptr = 0; AppId = StrJoin<char>(ptr0);
                    ptr += 10; ptr0 = ptr;
                    while (*++ptr && *ptr != '\n');
                    if (*ptr) {
                        *ptr = 0;
                        PassWord = StrJoin<char>(ptr0); file.close();
                        delete[] str; return true;
                    }
                }
            }
            delete[] str;
        }
        else
        {
            qout << "文件没有utf-8 bom头";
        }
        file.close();
    }
    return false;
}


void VARBOX::initFile()
{
    QString file = "SpeedBox.ini"; qout << "配置文件目录" << file;
    QString picfolder = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    QDir dir;
    if (!QFile::exists(file) && QFile::exists("SpeedBox2.ini"))
    {
        QFile::rename("SpeedBox2.ini", file);
    }
    if (!QFile::exists(file))
    {
        qout << "SpeedBox.ini 文件不存在";
        goto label_1;
    }
    else
    {
        QSettings set(file, QSettings::IniFormat);
        set.setIniCodec(QTextCodec::codecForName("UTF-8"));
        set.beginGroup("SpeedBox");
        QByteArray x = set.value("Version").toByteArray();
        set.endGroup();
        qout << "文件存在，版本信息为" << x;
        if (VARBOX::versionBefore(x, "21.8.1"))
        {
            qout << "ini文件过期，将其删除。";
            QFile::remove(file);
            goto label_1;
        }
        else
        {
            qout << "版本支持，继续读取";
            goto label_2;
        }
    }
label_1:
        {
            qout << "创建新的Ini文件。";
            NativeDir = picfolder;
            PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
            QSettings *IniWrite = new QSettings("SpeedBox.ini", QSettings::IniFormat);
            IniWrite->setIniCodec(QTextCodec::codecForName("UTF-8"));
            IniWrite->beginGroup("SpeedBox");
            IniWrite->setValue("Version", Version);
            IniWrite->endGroup();
            IniWrite->beginGroup("Wallpaper");
            IniWrite->setValue("PaperType", static_cast<int>(PaperType));
            IniWrite->setValue("TimeInerval", TimeInterval);
            IniWrite->setValue("PageNum", PageNum);
            IniWrite->setValue("setNative", false);
            IniWrite->setValue("AutoChange", false);
            IniWrite->setValue("NativeDir", NativeDir);
            IniWrite->setValue("UserCommand", UserCommand);
            IniWrite->setValue("UseDateAsBingName", UseDateAsBingName);
            IniWrite->endGroup();

            IniWrite->beginGroup("Translate");
            IniWrite->setValue("EnableTranslater", false);
            IniWrite->setValue("AutoHide", true);
            IniWrite->endGroup();

            IniWrite->beginGroup("UI");
            IniWrite->setValue("ColorTheme", static_cast<int>(CurTheme));
            IniWrite->setValue("x", 100);
            IniWrite->setValue("y", 100);
            IniWrite->setValue("ControlDesktopIcon", (bool)ControlDesktopIcon);
            IniWrite->endGroup();

            IniWrite->beginGroup("Dirs");
            IniWrite->setValue("OpenDir", PathToOpen);

            IniWrite->endGroup(); delete IniWrite;
            qout << "创建新的ini文件完毕。";
            goto label_3;
        }
label_2:
        {
            qout << "开始读取设置。";
            QSettings *IniRead = new QSettings(file, QSettings::IniFormat);
            IniRead->setIniCodec(QTextCodec::codecForName("UTF-8"));
            IniRead->beginGroup("SpeedBox");
            IniRead->setValue("Version", Version);
            IniRead->endGroup();
            IniRead->beginGroup("Wallpaper");
            NativeDir = IniRead->value("NativeDir").toString();
            unsigned safeEnum = IniRead->value("PaperType").toInt();
            if (safeEnum>9) safeEnum = 0;
            PaperType = static_cast<PAPER_TYPE>(safeEnum);
            TimeInterval = IniRead->value("TimeInerval").toInt();
            PageNum = IniRead->value("PageNum").toInt();
            UserCommand = IniRead->value("UserCommand").toString();
            AutoChange = IniRead->value("AutoChange").toBool();
            if (IniRead->contains("AutoRotationBingPicture"))
            {
                UseDateAsBingName = IniRead->value("UseDateAsBingName").toBool();
                AutoSaveBingPicture = IniRead->value("AutoSaveBingPicture").toBool();
            }
            else
            {
                IniRead->setValue("AutoSaveBingPicture", AutoSaveBingPicture);
                IniRead->setValue("UseDateAsBingName", UseDateAsBingName);
            }
            if (IniRead->contains("FirstChange"))
            {
                FirstChange = IniRead->value("FirstChange").toBool();
            }
            else
                IniRead->setValue("FirstChange", FirstChange);
            IniRead->endGroup();
            qout << "读取壁纸信息完毕";
            IniRead->beginGroup("Translate");
            AutoHide = IniRead->value("AutoHide").toBool();
            EnableTranslater = IniRead->value("EnableTranslater").toBool();
            IniRead->endGroup();
            qout << "读取翻译信息完毕";
            IniRead->beginGroup("Dirs");
            PathToOpen = IniRead->value("OpenDir").toString();
            IniRead->endGroup();
            qout << "读取路径信息完毕";
            IniRead->beginGroup("UI");
            if (IniRead->contains("ControlDesktopIcon"))
            {
                if (IniRead->value("ControlDesktopIcon").toBool())
                    ControlDesktopIcon = new DesktopMask;
            }
            else
                IniRead->setValue("ControlDesktopIcon", false);  //enableUSBhelper

            safeEnum = IniRead->value("ColorTheme").toInt();
            if (safeEnum > 7) safeEnum = 0;
            CurTheme = static_cast<COLOR_THEME>(safeEnum);
            IniRead->endGroup();
            IniRead->beginGroup("USBhelper");
            enableUSBhelper = IniRead->value("enableUSBhelper", enableUSBhelper).toBool();
            IniRead->endGroup();
            delete IniRead;
            qout << "读取设置完毕。";
        }
label_3:
        {
            if (!dir.exists(PathToOpen) && !dir.mkdir(PathToOpen))
            {
                PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
                sigleSave("Dirs", "OpenDir", PathToOpen);
            }
            HaveAppRight = check_app_right();
            EnableTranslater = HaveAppRight && EnableTranslater;
        }
    const std::string apifile = "WallpaperApi.json";
    if (!QFile::exists(apifile.c_str()))
    {
        qout << "文件不存在!";
        picfolder += "\\桌面壁纸";
        dir.mkdir(picfolder);
        dir.cd(picfolder);
        QFile::copy(":/json/WallpaperApi.json", (apifile+".temp").c_str());
        const std::vector<QString> lst {"最热壁纸", "风景壁纸", "动漫壁纸", "极简壁纸", "随机壁纸", "鬼刀壁纸", "必应壁纸"};
        for (const auto&c: lst)
            dir.mkdir(c);
        std::vector<QString>::const_iterator iter = lst.begin();
        YJson json(apifile+".temp", YJson::UTF8);
        for (auto&c: json["Default"]["ApiData"])
            c["Folder"].setText((picfolder+"\\"+*iter++).toUtf8());
        for (auto&c: json["User"]["ApiData"])
            c["Folder"].setText((picfolder+"\\"+*iter).toUtf8());
        json["BingApi"]["Folder"].setText((picfolder+"\\"+*++iter).toUtf8());
        for (auto&c: json["OtherApi"]["ApiData"])
        {
            c["Folder"].setText((picfolder+"\\"+c.getKeyString()).toUtf8());
            dir.mkdir(c["Folder"].getValueString());
        }
        json.toFile(apifile, YJson::UTF8, true);
        QFile::remove((apifile+".temp").c_str());
    }
}

void VARBOX::initChildren()
{
    qout << "开始创建基本成员";
    wallpaper = new Wallpaper;                           // 创建壁纸更换对象
    qout << "悬浮窗";
    Form* form = new Form;
    qout << "基本成员创建完毕!";
    static APPBARDATA abd { 0,0,0,0,{0,0,0,0},0 };
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = HWND(form->winId());
    abd.uCallbackMessage = MSG_APPBAR_MSGID;
    SHAppBarMessage(ABM_NEW, &abd);
    QObject::connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged, form, [this, form](const QRect&rect){
        RECT rt1; int w, h;
        GetWindowRect((HWND)(this->form->winId()), &rt1);
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);

        RECT rt2; GetWindowRect(HWND(VarBox->form->winId()), &rt2);
        this->form->move(this->form->pos().x() * rect.width() / VarBox->ScreenWidth, this->form->pos().y() * rect.height() / VarBox->ScreenHeight);
        *const_cast<int*>(&(VarBox->SysScreenWidth)) = w;
        *const_cast<int*>(&(VarBox->SysScreenHeight)) = h;
        *const_cast<int*>(&(VarBox->ScreenWidth)) = rect.width();
        *const_cast<int*>(&(VarBox->ScreenHeight)) = rect.height();
        if (ControlDesktopIcon)
        {
            ControlDesktopIcon->left->move(0, VarBox->ScreenHeight/2-5);
            ControlDesktopIcon->right->move(VarBox->ScreenWidth-1, VarBox->ScreenHeight/2-6);
        }
        form->keepInScreen();
    });
}

void VARBOX::initBehaviors()
{
    form->show();                                                              //显示悬浮窗
    form->keepInScreen();
}

void VARBOX::sigleSave(QString group, QString key, QString value)
{
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup(group);
    IniWrite.setValue(key, value);
    IniWrite.endGroup();
}

wchar_t* VARBOX::runCmd(const QString& program, const QStringList& argument, short line)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(argument);
    connect(&process, &QProcess::errorOccurred, [&](){qout << "运行出错"; line=0;});
    process.start();
    process.waitForStarted(); //等待程序启动
    process.waitForFinished(15000);
    if (line) {
        for (int c = 1; c < line; c++)
        {
            process.readLine();
        }

        return GetCorrectUnicode(process.readLine());
    }
    else return nullptr;
}

void VARBOX::openDirectory(const QString& dir)
{
#ifdef Q_OS_WIN
    ShellExecuteW(NULL, L"open", L"explorer.exe", QDir::toNativeSeparators(dir).toStdWString().c_str(), NULL, SW_SHOWNORMAL);
#elif def Q_OS_LINUX
#endif
}

BOOL VARBOX::SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor)    //设置窗口WIN10风格
{
    typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(HWND, struct WINDOWCOMPOSITIONATTRIBDATA*);
    pfnSetWindowCompositionAttribute pSetWindowCompositionAttribute = NULL;
    if (mode == ACCENT_STATE::ACCENT_DISABLED)
    {
        //		if (bAccentNormal == FALSE)
        {
            SendMessageA(hWnd, WM_THEMECHANGED, 0, 0);
            //			bAccentNormal = TRUE;
        }
        return TRUE;
    }
    //	bAccentNormal = FALSE;
    BOOL ret = FALSE;
    HMODULE hUser = GetModuleHandleA("user32.dll");
    if (hUser)
        pSetWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
    if (pSetWindowCompositionAttribute)
    {
        ACCENT_POLICY accent = { mode, 2, AlphaColor, 0 };
        WINDOWCOMPOSITIONATTRIBDATA data;
        data.Attrib = WINDOWCOMPOSITIONATTRIB::WCA_ACCENT_POLICY;
        data.pvData = &accent;
        data.cbData = sizeof(accent);
        ret = pSetWindowCompositionAttribute(hWnd, &data);
    }
    return ret;
}

inline void _getINT(const char* &X, int &x)
{
    while (true)
    {
        x += *X - '0';
        ++X;
        if (*X == '.' || *X == '\0')
            return;
        x *= 10;
    }
}
bool VARBOX::versionBefore(const char* A, const char* B)
{
    int a=0, b=0;
    _getINT(A, a); _getINT(B, b);
    ++A; ++B;
    if (a<b) return true; else if (a>b) return false;
    a = b = 0;
    _getINT(A, a); _getINT(B, b);
    ++A; ++B;
    if (a<b) return true; else if (a>b) return false;
    a = b = 0;
    _getINT(A, a); _getINT(B, b);
    if (a<b) return true; else return false;
}

void VARBOX::MSG(const char *text, const char* title, QMessageBox::StandardButtons s)
{
    QMessageBox message(QMessageBox::Information, title, text, s, NULL);
    QFile qss(":/qss/dialog_style.qss");
    qss.open(QFile::ReadOnly);
    message.setStyleSheet(QString(qss.readAll()));
    qss.close();
    message.exec();
}
