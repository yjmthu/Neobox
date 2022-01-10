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
#include <QDesktopServices>

#include "funcbox.h"
#include "YString.h"
#include "YJson.h"
#include "form.h"
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


void VARBOX::initFile()
{
    constexpr char file[] = "SpeedBox.ini"; qout << "配置文件目录" << file;
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
        if (getVersion(x)<getVersion("21.8.1"))
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
            QSettings *IniWrite = new QSettings("SpeedBox.ini", QSettings::IniFormat);
            IniWrite->setIniCodec(QTextCodec::codecForName("UTF-8"));
            IniWrite->beginGroup("SpeedBox");
            IniWrite->setValue("Version", Version);
            IniWrite->endGroup();
            IniWrite->beginGroup("Wallpaper");
            IniWrite->setValue("PaperType", 0);
            IniWrite->setValue("TimeInerval", 15);
            IniWrite->setValue("PageNum", 1);
            IniWrite->setValue("setNative", false);
            IniWrite->setValue("AutoChange", false);
            IniWrite->setValue("NativeDir", picfolder);
            IniWrite->setValue("UserCommand", "python.exe -u X:\\xxxxx.py");
            IniWrite->setValue("UseDateAsBingName", true);
            IniWrite->endGroup();

            IniWrite->beginGroup("Translate");
            IniWrite->setValue("EnableTranslater", false);
            IniWrite->setValue("AutoHide", true);
            IniWrite->endGroup();

            IniWrite->beginGroup("UI");
            IniWrite->setValue("ColorTheme", static_cast<int>(CurTheme));
            IniWrite->setValue("x", 100);
            IniWrite->setValue("y", 100);
            IniWrite->endGroup();

            IniWrite->beginGroup("Dirs");
            IniWrite->setValue("OpenDir", QDir::toNativeSeparators(qApp->applicationDirPath()));

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

            unsigned safeEnum = IniRead->value("ColorTheme").toInt();
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

char _getINT(const char* &X)
{
    unsigned char x = 0;
    while (true)
    {
        x += *X++ - '0';
        if (*X == '.' || *X == '\0')
            return x;
        x *= 10;
    }
    return x;
}
uint32_t VARBOX::getVersion(const char* A)
{
    uint32_t buffer = _getINT(A);
    (buffer <<= 8) |= _getINT(++A);
    (buffer <<= 8) |= _getINT(++A);
    return buffer;
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
