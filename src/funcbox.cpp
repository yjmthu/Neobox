﻿#include <fstream>
#include <oaidl.h>
#include <WinInet.h>
#include <QApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QDir>

#include "funcbox.h"
#include "YString.h"
#include "form.h"

#pragma comment(lib,"wininet.lib")


unsigned char GetNtVersionNumbers()
{
    HMODULE hModNtdll= NULL;
    DWORD dwMajorVer = 0, dwMinorVer = 0, dwBuildNumber = 0;
    if (hModNtdll= LoadLibraryA("ntdll.dll"))
    {
        typedef void (WINAPI *pfRTLGETNTVERSIONNUMBERS)(DWORD*,DWORD*, DWORD*);
        pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;
        pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
        if (pfRtlGetNtVersionNumbers)
        {
           pfRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer,&dwBuildNumber);
           dwBuildNumber&= 0x0ffff;
        }

        FreeLibrary(hModNtdll);
    }
    if (dwMajorVer>10)
        return 11;
    else if (dwMajorVer==10)
    {
        if (dwMinorVer==0)
        {
            if (dwBuildNumber >= 21996)
                return 11;
            else
                return 10;
        }
        else if (dwMinorVer > 0)
            return 11;
    }
    else if (dwMajorVer==7)
    {
        if (dwMinorVer == 1)
            return 7;
    }
    return 0;
}

wchar_t* get_reg_paper()
{
    QSettings set("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    if (set.contains("WallPaper"))
    {
        return StrJoin(set.value("WallPaper").toString().toStdWString().c_str());
    }
    return nullptr;
}

VARBOX* VarBox = nullptr;
HANDLE VARBOX::HMutex = NULL;

VARBOX::VARBOX(int w, int h):
    WinVersion(GetNtVersionNumbers()), ScreenWidth(w), ScreenHeight(h),
    hOleacc(LoadLibrary("oleacc.dll")), hIphlpapi(LoadLibrary("iphlpapi.dll")), hDwmapi(LoadLibrary("dwmapi.dll"))
{
    qout << "VarBox构造函数开始。";
    loadFunctions();
    wchar_t* temp_paper = get_reg_paper();
    if (temp_paper)
        PicHistory.push_back(std::pair<bool, void*>(true, temp_paper));
    CurPic = PicHistory.begin();
    QString file = get_ini_path(); short type = 0; qout << "配置文件目录" << file;
    while (true)
    {
        if (type == 0)
        {
            if (!QFile::exists(file))
            {
                qout << "SpeedBox.ini 文件不存在";
                sigleSave("SpeedBox", "Version", Version);
                type = 1;
                continue;
            }
            else
            {
                //char t[12] = { 0 };
                //GetPrivateProfileString("SpeedBox", "Version", t, t, 11, file.toStdString().c_str());
                QSettings set(file, QSettings::IniFormat);
                set.beginGroup("SpeedBox");
                QByteArray x = set.value("Version").toByteArray();
                set.endGroup();
                qout << "文件存在，版本信息为" << x;
                if (VARBOX::versionBefore(x, "21.8.1"))
                {
                    qout << "ini文件过期，将其删除。";
                    QFile::remove(file); sigleSave("SpeedBox", "Version", Version);
                    type = 1;
                    continue;
                }
                else
                {
                    qout << "版本支持，继续读取";
                    type = 2;
                    continue;
                }
            }
        }
        if (type == 1)
        {
            qout << "创建新的Ini文件。";
            FamilyPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
            NativeDir = FamilyPath;
            QSettings *IniWrite = new QSettings(get_ini_path(), QSettings::IniFormat);
            IniWrite->beginGroup("Wallpaper");
            IniWrite->setValue("PaperType", static_cast<int>(PaperType));
            IniWrite->setValue("TimeInerval", TimeInterval);
            IniWrite->setValue("PageNum", PageNum);
            IniWrite->setValue("setNative", false);
            IniWrite->setValue("AutoChange", false);
            IniWrite->setValue("NativeDir", FamilyPath);
            IniWrite->setValue("UserCommand", UserCommand);
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
            IniWrite->setValue("FamilyPath", FamilyPath);
            IniWrite->setValue("OpenDir", PathToOpen);

            for (short i = 0; i < 10; i++)
                IniWrite->setValue(PaperTypes[i][0], FamilyNames[i]);
            IniWrite->endGroup(); delete IniWrite;
            saveTrayStyle();
            type = 3;
            qout << "创建新的ini文件完毕。";
            continue;
        }
        else if (type == 2)
        {
            qout << "开始读取设置。";
            QSettings *IniRead = new QSettings(file, QSettings::IniFormat);
            IniRead->beginGroup("Wallpaper");
            NativeDir = IniRead->value("NativeDir").toString();
            unsigned safeEnum = IniRead->value("PaperType").toInt();
            if (safeEnum>9) safeEnum = 0;
            PaperType = static_cast<PAPER_TYPE>(safeEnum);
            TimeInterval = IniRead->value("TimeInerval").toInt();
            PageNum = IniRead->value("PageNum").toInt();
            UserCommand = IniRead->value("UserCommand").toString();
            AutoChange = IniRead->value("AutoChange").toBool();
            IniRead->endGroup();
            qout << "读取壁纸信息完毕";
            IniRead->beginGroup("Translate");
            AutoHide = IniRead->value("AutoHide").toBool();
            EnableTranslater = IniRead->value("EnableTranslater").toBool();
            IniRead->endGroup();
            qout << "读取翻译信息完毕";
            IniRead->beginGroup("Dirs");
            PathToOpen = IniRead->value("OpenDir").toString();
            FamilyPath = IniRead->value("FamilyPath").toString();
            for (short i = 0; i < 10; i++)
            {
                FamilyNames[i] = IniRead->value(PaperTypes[i][0]).toString();
            }
            IniRead->endGroup();
            qout << "读取路径信息完毕";
            IniRead->beginGroup("UI");
            safeEnum = IniRead->value("ColorTheme").toInt();
            if (safeEnum > 3) safeEnum = 0;
            CurTheme = static_cast<COLOR_THEME>(safeEnum);
            IniRead->endGroup();
            delete IniRead;
            qout << "开始读取风格";
            readTrayStyle();
            qout << "读取设置完毕。";
            type = 3;
            continue;
        }
        else if (type == 3)
        {
            for (short i = 0; i < 7; i++)
                get_pic_path(i);
            if (!get_son_dir(PathToOpen))
            {
                PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
                sigleSave("Dirs", "OpenDir", PathToOpen);
            }
            HaveAppRight = check_app_right();
            EnableTranslater = HaveAppRight && EnableTranslater;
            break;
        }
    }
    qout << "VarBox构造函数结束。";
}

VARBOX::~VARBOX()
{
    qout << "结构体析构中~";
    delete form;
    for (auto x: PicHistory)
    {
        if (x.first)
        {
            delete [] static_cast<wchar_t*>(x.second);
        }
        else
        {
            delete [] static_cast<char*>(x.second);
        }
    }
    if (AppId) delete [] AppId;
    if (PassWord) delete [] PassWord;
    if (hIphlpapi) FreeLibrary(hIphlpapi);
    if (hOleacc) FreeLibrary(hOleacc);
    if (hDwmapi) FreeLibrary(hDwmapi);
    qout << "结构体析构成功。";
}

void VARBOX::loadFunctions()
{
    if (!WinVersion)
    {
        qApp->exit(RETCODE_ERROR_EXIT);
        return;
    }
    if (hOleacc)
    {
        AccessibleObjectFromWindow = (pfnAccessibleObjectFromWindow)GetProcAddress(hOleacc, "AccessibleObjectFromWindow");
        AccessibleChildren = (pfnAccessibleChildren)GetProcAddress(hOleacc, "AccessibleChildren");
    }

    if (hIphlpapi)           //获取两个函数指针
    {
        GetAdaptersAddresses = (pfnGetAdaptersAddresses)GetProcAddress(hIphlpapi, "GetAdaptersAddresses");
        GetIfTable = (pfnGetIfTable)GetProcAddress(hIphlpapi, "GetIfTable");
    }

    if (hDwmapi)
    {
        pDwmGetWindowAttribute = (pfnDwmGetWindowAttribute)GetProcAddress(hDwmapi, "DwmGetWindowAttribute");
    }


    if (!(GetIfTable && GetAdaptersAddresses && AccessibleObjectFromWindow && AccessibleChildren && pDwmGetWindowAttribute))
    {
        qApp->exit(RETCODE_ERROR_EXIT);
    }
}

bool VARBOX::check_app_right()
{
    QString appid_path = get_dat_path() + "\\AppId.txt";
    if (!QFile::exists(appid_path)) return false;
    std::ifstream file(appid_path.toStdString(), std::ios::in | std::ios::binary);
    if( file.is_open())
    {
        qout << "成功打开密钥文件";
        file.seekg(0, std::ios::end );
        int size = file.tellg();
        if (size < 20)
        {
            file.close();
            return false;
        }
        unsigned char c1 = 0, c2 = 0, c3 = 0; file.seekg(0,std::ios::beg);
        if (!(file.read((char*)&c1, sizeof(char))
                &&
              file.read((char*)&c2, sizeof(char))
                &&
              file.read((char*)&c3, sizeof(char))
           ))
        {
            qout << "文件读取失败1";
            file.close();
            return false;
        }
        size -= sizeof(char) * 3;
        if (c1 == 0xef && c2 == 0xbb && c3 == 0xbf)
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
                    *ptr = 0; AppId = StrJoin(ptr0);
                    ptr += 10; ptr0 = ptr;
                    while (*++ptr && *ptr != '\n');
                    if (*ptr) {
                        *ptr = 0;
                        PassWord = StrJoin(ptr0); file.close();
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

void VARBOX::readTrayStyle()
{
    QSettings IniRead(get_ini_path(), QSettings::IniFormat);
    IniRead.beginGroup("TaskStyle");
    unsigned long long safeEnum = IniRead.value("aMode_f").toUInt();
    if (safeEnum > 6 && safeEnum != 150) safeEnum = 0;
    aMode[0] = static_cast<ACCENT_STATE>(safeEnum);
    safeEnum = IniRead.value("aMode_s").toUInt();
    if (safeEnum > 6 && safeEnum != 150) safeEnum = 0;
    aMode[1] = static_cast<ACCENT_STATE>(safeEnum);
    qout << "读取中。。";
    safeEnum = IniRead.value("dAlphaColor_f").toULongLong();
    if (safeEnum > 0xffffffff) safeEnum = 0x11111111;
    dAlphaColor[0] = safeEnum;
    safeEnum = IniRead.value("dAlphaColor_s").toULongLong();
    if (safeEnum > 0xffffffff) safeEnum = 0x11111111;
    dAlphaColor[1] = safeEnum;
    safeEnum = IniRead.value("bAlpha_f").toUInt();
    if (safeEnum > 0xff) safeEnum = 0xff;
    bAlpha[0] = safeEnum;
    safeEnum = IniRead.value("bAlpha_s").toUInt();
    if (safeEnum > 0xff) safeEnum = 0xff;
    bAlpha[1] = safeEnum;
    safeEnum = IniRead.value("RefreshTime").toInt();
    if (safeEnum<33||safeEnum>198) safeEnum = 33;
    RefreshTime = safeEnum;
    safeEnum = IniRead.value("iPos").toInt();
    if (safeEnum > 2) safeEnum = 0;
    iPos = static_cast<TaskBarCenterState>(safeEnum);
    IniRead.endGroup();
}

void VARBOX::saveTrayStyle()
{
    QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("TaskStyle");
    IniWrite.setValue("aMode_f", static_cast<int>(aMode[0]));
    IniWrite.setValue("aMode_s", static_cast<int>(aMode[1]));
    IniWrite.setValue("dAlphaColor_f", static_cast<unsigned long long>(dAlphaColor[0]));
    IniWrite.setValue("dAlphaColor_s", static_cast<unsigned long long>(dAlphaColor[1]));
    IniWrite.setValue("bAlpha_f", static_cast<int>(bAlpha[0]));
    IniWrite.setValue("bAlpha_s", static_cast<int>(bAlpha[1]));
    IniWrite.setValue("RefreshTime", RefreshTime);
    IniWrite.setValue("iPos",  static_cast<int>(iPos));
    IniWrite.endGroup();
}

QString VARBOX::get_wal_path()
{
    QString str;
    if (FamilyPath.isEmpty() || !get_son_dir(FamilyPath))
    {
        qout << "图片目录为空" << FamilyPath;
        FamilyPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    }
    str = FamilyPath + "\\" + FamilyNames[7];
    if (FamilyNames[7].isEmpty() || !get_son_dir(str))
    {
        FamilyNames[7] = PaperTypes[7][1];
        str = FamilyPath + "\\" + FamilyNames[7];
    }
    QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Dirs");
    IniWrite.setValue("FamilyPath", FamilyPath);
    IniWrite.setValue(PaperTypes[7][0], FamilyNames[7]);
    IniWrite.endGroup();
    return str;
}

QString VARBOX::get_pic_path(short i)
{
    qout << "获取图片路径开始" << i;
    QString str = get_wal_path() + "\\" + FamilyNames[i];
    if (FamilyNames[i].isEmpty() || !get_son_dir(str))
    {
        qout << "FamilyPath为空或者无法创建！";
        FamilyNames[i] = PaperTypes[i][1];
        str = FamilyPath + "\\", FamilyNames[7] + "\\" + FamilyNames[i];
        get_son_dir(str);
    }
    QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Dirs");
    IniWrite.setValue(PaperTypes[i][0], FamilyNames[i]);
    IniWrite.endGroup();
    qout << "最终图片路径结果：" << str;
    return str;
}

QString VARBOX::get_ini_path()
{
    return get_dat_path() + "\\SpeedBox2.ini";
}

QString VARBOX::get_dat_path()
{
    QString str = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +  "/AppData/Local/SpeedBox");
    get_son_dir(str);
    return str;
}

bool VARBOX::get_son_dir(QString str)
{
    QDir dir;
    return dir.exists(str) || dir.mkdir(str);
}

void VARBOX::sigleSave(QString group, QString key, QString value)
{
    QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup(group);
    IniWrite.setValue(key, value);
    IniWrite.endGroup();
}

bool VARBOX::getWebCode(const char* url, std::string& src, bool auto_delete)
{
    src.clear();
    HINTERNET hSession = InternetOpen("Firefox", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hSession != NULL)
    {
        HINTERNET hURL = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
        if (auto_delete) delete[] url;
        if (hURL)
        {
            char temp[1025] = { 0 };
            DWORD dwRecv = 1;
            while (dwRecv > 0 && VarBox->RunApp)
            {
                memset(temp, 0, 1024);
                InternetReadFile(hURL, temp, 1024, &dwRecv);
                src += temp;
            }
            InternetCloseHandle(hURL);
            hURL = NULL;
        }

        InternetCloseHandle(hSession);
        hSession = NULL;
    }
    else
    {
        if (auto_delete) delete[] url;
    }
    return VarBox->RunApp && !src.empty();
}

bool VARBOX::getBingCode(std::string& code)
{
    HINTERNET hSession = InternetOpen("Chromium", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hSession)
    {
        HINTERNET hURL = InternetOpenUrl(hSession, "https://cn.bing.com/", NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
        if (hURL)
        {
            char temp[513] = { 0 };
            DWORD dwRecv = 1;
            InternetReadFile(hURL, temp, 512, &dwRecv);
            code += temp;
            memset(temp, 0, 512);
            InternetReadFile(hURL, temp, 512, &dwRecv);
            code += temp;
            memset(temp, 0, 512);
            InternetReadFile(hURL, temp, 512, &dwRecv);
            code += temp;
            InternetCloseHandle(hURL);
            hURL = NULL;
        }
        InternetCloseHandle(hSession);
        hSession = NULL;
    }
    return !code.empty();
}

// 会删除url
bool VARBOX::getTransCode(const char* url, std::string& outcome)
{
    //qout << "翻译请求：" << url << Qt::endl;
    HINTERNET hSession = InternetOpenA("Microsoft Edge", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hSession)
    {
        HINTERNET hURL = InternetOpenUrlA(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
        delete [] url; //qout << "发送翻译请求成功。";
        if (hURL)
        {
            char buffer[1025] = { 0 }; DWORD dwRecv = 0;
            do {
                memset(buffer, 0, 1024);
                InternetReadFile(hURL, buffer, 1024, &dwRecv);
                if (dwRecv)
                {
                    buffer[dwRecv] = 0;
                    //qout << "读入的字节数量：" << dwRecv;
                    outcome.append(buffer);
                }
                else
                    break;
            } while (VarBox->RunApp);
            InternetCloseHandle(hURL);
        }

        InternetCloseHandle(hSession);
        hSession = NULL;
    }
    else
    {
        delete [] url;
    }
    return !outcome.empty();
}

bool VARBOX::downloadImage(const char* url, const QString path, bool auto_delete)
{
    if (QFile::exists(path)) return 1;
    bool OK = false;
    DWORD dwRecv = 0;
    DWORD allWrite = 0;
    char* szDownLoad = new char[1024 * 40];
    HINTERNET hSession = InternetOpen("Chromium", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hSession)
    {
        HINTERNET hOpenUrl = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (auto_delete) delete [] url;
        if (hOpenUrl)
        {
            qout << "成功打开网址";
            QFile file(path);
            if (file.open(QIODevice::WriteOnly))
            {
                qout << "开始写入图片文件";
                while (VarBox->RunApp)
                {
                    //memset(szDownLoad, 0, 1024 * 40 * sizeof(char));
                    Sleep(10); // qout << "下载循环中";
                    InternetReadFile(hOpenUrl, szDownLoad, 1024 * 40, &dwRecv);
                    if (!dwRecv)
                        break;
                    file.write(szDownLoad, dwRecv);
                    allWrite += dwRecv;
                }
                qout << "下载结束";
                file.close();
                OK = (allWrite >= 16000) && VarBox->RunApp;
                if (!OK && QFile::exists(path))
                {
                    QFile::remove(path);
                    qout << "删除错误文件";
                }
                else
                {
                    qout << "下载成功！";
                }
            }
            InternetCloseHandle(hOpenUrl);
        }
        InternetCloseHandle(hSession);
    }
    else
    {
        if (auto_delete) delete [] url;
    }
    delete[] szDownLoad;
    qout << "退出函数" << OK;
    return OK;
}

char* VARBOX::runCommand(QString program, QStringList argument, short line)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(argument);
    process.start();
    process.waitForStarted(); //等待程序启动
    process.waitForFinished(5000);
    if (line) {
        for (int c = 1; c < line; c++)
            process.readLine();
        return StrJoin((const char*)process.readLine());
    }
    else return nullptr;
}

bool VARBOX::isOnline(bool wait)
{
    qout << "检测网络连接中。";
    DWORD flag;
    for (int c = 1; VarBox->RunApp && (c <= (wait ? 30 : 1)); c++)
    {
        if (VarBox->RunApp && InternetGetConnectedState(&flag, 0))
        {
            qout << "网络连接正常。";
            return true;
        }
        else if (VarBox->RunApp)
            for (short j = 1; VarBox->RunApp && (j <= 6); j++)
                Sleep(500);
        qout << "再次检测网络连接。";
    }
    qout << "没有网络连接。";
    return false;
}

BOOL VARBOX::SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor)    //设置窗口WIN10风格
{
    typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(HWND, struct WINDOWCOMPOSITIONATTRIBDATA*);
    pfnSetWindowCompositionAttribute pSetWindowCompositionAttribute = NULL;
    if (mode == ACCENT_STATE::ACCENT_DISABLED)
    {
        //		if (bAccentNormal == FALSE)
        {
            SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
            //			bAccentNormal = TRUE;
        }
        return TRUE;
    }
    //	bAccentNormal = FALSE;
    BOOL ret = FALSE;
    HMODULE hUser = GetModuleHandle("user32.dll");
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

BOOL VARBOX::PathFileExists(LPCSTR pszPath)
{
    typedef BOOL(WINAPI* pfnPathFileExists)(LPCSTR pszPath);
    pfnPathFileExists pPathFileExists = NULL;
    BOOL ret = FALSE;
    HMODULE hUser = GetModuleHandle(TEXT("Shlwapi.dll"));
    if (hUser)
        pPathFileExists = (pfnPathFileExists)GetProcAddress(hUser, "PathFileExistsA");
    if (pPathFileExists)
    {
        qout << "找到函数！";
        ret = pPathFileExists(pszPath);
    }
    else
    {
        qout << "找不到函数！";
    }
    return ret;
}

BOOL VARBOX::PathFileExists(LPWSTR pszPath)
{
    typedef BOOL(WINAPI* pfnPathFileExists)(LPWSTR pszPath);
    pfnPathFileExists pPathFileExists = NULL;
    BOOL ret = FALSE;
    HMODULE hUser = GetModuleHandle(TEXT("Shlwapi.dll"));
    if (hUser)
        pPathFileExists = (pfnPathFileExists)GetProcAddress(hUser, "PathFileExistsW");
    if (pPathFileExists)
    {
        qout << "找到函数！";
        ret = pPathFileExists(pszPath);
    }
    else
    {
        qout << "找不到函数！";
    }
    return ret;
}
