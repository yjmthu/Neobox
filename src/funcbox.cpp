#include <fstream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <oaidl.h>
#include <WinInet.h>
#include <shlobj.h>
#include <QApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include "funcbox.h"
#include "YString.h"

#pragma comment(lib,"wininet.lib")

VAR_BOX VarBox = {
    {
        {"Latest", "最新壁纸"}, {"Hot", "最热壁纸"}, {"Nature", "风景壁纸"},{"Anime", "动漫壁纸"},
        {"Simple", "极简壁纸"}, {"Random", "随机壁纸"},{"Bing", "必应壁纸"},
        {"Wallpapers", "桌面壁纸"}, {"Native", "本地壁纸"},{"Advance", "高级壁纸"}
    },
    {
        "最新壁纸","最热壁纸","风景壁纸","动漫壁纸","极简壁纸",
        "随机壁纸","必应壁纸","桌面壁纸","本地壁纸","高级壁纸"
    },
    "", PAPER_TYPE::Latest, COLOR_THEME::ClassicWhite, true, false, false, 1, 15, "",
    "X:\\xxx\\python.exe -u X:\\xxx.py", "", 0, 0, false, false, nullptr, nullptr, false, false,
    { ACCENT_STATE::ACCENT_DISABLED, ACCENT_STATE::ACCENT_DISABLED }, { 0x11111111, 0x11111111 },
    /* 背景 */ {0xff, 0xff},/* 图标 */ TaskBarCenterState::TASK_LEFT, 33, NULL, NULL, NULL, nullptr
};

namespace FuncBox {

    bool IsDirExist(QString csDir)
    {
        QDir dir;
        return dir.exists(csDir);
    }

    void sigleSave(QString group, QString key, QString value)
    {
        QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup(group);
        IniWrite.setValue(key, value);
        IniWrite.endGroup();
    }

    bool getWebCode(const char* url, std::string& src)
    {
        bool OK = false;  src.clear();
        HINTERNET hSession = InternetOpen("Firefox", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession != NULL)
        {
            HINTERNET hURL = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
            delete[] url;
            if (hURL != NULL)
            {
                char Temp[1024] = { 0 };
                ULONG Number = 1;
                while (Number > 0)
                {
                    InternetReadFile(hURL, Temp, 1023, &Number);
                    src += Temp;
                }
                OK = !src.empty();
                InternetCloseHandle(hURL);
                hURL = NULL;
            }

            InternetCloseHandle(hSession);
            hSession = NULL;
        }
        return OK;
    }

    bool getBingCode(std::string& code)
    {
        HINTERNET hSession = InternetOpen("Chromium", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hURL = InternetOpenUrl(hSession, "https://cn.bing.com/", NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
            if (hURL)
            {
                CHAR Temp[512] = { 0 };
                ULONG Number = 1;
                InternetReadFile(hURL, Temp, 511, &Number);
                code += Temp;
                InternetCloseHandle(hURL);
                hURL = NULL;
            }
            InternetCloseHandle(hSession);
            hSession = NULL;
        }
        return !code.empty();
    }

    // 会删除url
    bool getTransCode(const char* url, std::string* outcome)
    {
        HINTERNET hSession = InternetOpenA("Microsoft Edge", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hURL = InternetOpenUrlA(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
            delete [] url; qout << "发送翻译请求成功。";
            if (hURL)
            {
                char buffer[1025]; DWORD dwRecv = 0;
                do {
                    ZeroMemory(buffer, 1024);
                    InternetReadFile(hURL, buffer, 1024, &dwRecv);
                    if (dwRecv)
                    {
                        buffer[dwRecv] = 0;
                        qout << "读入的字节数量：" << dwRecv;
                        outcome->append(buffer);
                    }
                    else
                        break;
                } while (true);
                InternetCloseHandle(hURL);
            }

            InternetCloseHandle(hSession);
            hSession = NULL;
            qout << 3;
        }
        return !outcome->empty();
    }

    bool downloadImage(const char* url, const QString path)
    {
        if (QFile::exists(path)) return 1;
        bool OK = false;
        DWORD dwRecv = 0;
        int allWrite = 0;
        char* szDownLoad = new char[1024 * 40];
        memset(szDownLoad, 0, 1024 * 40 * sizeof(char));
        HINTERNET hSession = InternetOpen("Chromium", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hOpenUrl = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
            qout << "删除图片链接：" << url; delete [] url;
            if (hOpenUrl)
            {
                qout << "成功打开网址";
                QFile file(path);
                if (file.open(QIODevice::WriteOnly))
                {
                    qout << "开始写入图片文件";
                    while (true)
                    {
                        Sleep(10); // qout << "下载循环中";
                        InternetReadFile(hOpenUrl, szDownLoad, 1024 * 40, &dwRecv);
                        if (!dwRecv)
                            break;
                        file.write(szDownLoad, dwRecv);
                        allWrite += dwRecv;
                    }
                    qout << "下载结束";
                    file.close(); OK = (allWrite != 0) && (allWrite != 235);
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
            }
        }
        delete[] szDownLoad;
        qout << "退出函数" << OK;
        return OK;
    }

    bool get_son_dir(QString str)
    {
        QDir dir;
        return dir.exists(str) || dir.mkdir(str);
    }

    // 获取壁纸文件夹
    QString get_wal_path()
    {
        QString str;
        if (VarBox.FamilyPath.isEmpty() || !get_son_dir(VarBox.FamilyPath))
        {
            qout << "图片目录为空" << VarBox.FamilyPath;
            VarBox.FamilyPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        }
        str = VarBox.FamilyPath + "\\" + VarBox.FamilyNames[7];
        if (VarBox.FamilyNames[7].isEmpty() || !get_son_dir(str))
        {
            VarBox.FamilyNames[7] = VarBox.PaperTypes[7][1];
            str = VarBox.FamilyPath + "\\" + VarBox.FamilyNames[7];
        }
        QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup("Dirs");
        IniWrite.setValue("FamilyPath", VarBox.FamilyPath);
        IniWrite.setValue(VarBox.PaperTypes[7][0], VarBox.FamilyNames[7]);
        IniWrite.endGroup();
        return str;
    }

    QString get_pic_path(short i)
    {
        qout << "获取图片路径开始" << i;
        QString str = get_wal_path() + "\\" + VarBox.FamilyNames[i];
        if (VarBox.FamilyNames[i].isEmpty() || !get_son_dir(str))
        {
            qout << "FamilyPath为空或者无法创建！";
            VarBox.FamilyNames[i] = VarBox.PaperTypes[i][1];
            str = VarBox.FamilyPath + "\\", VarBox.FamilyNames[7] + "\\" + VarBox.FamilyNames[i];
            get_son_dir(str);
        }
        QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup("Dirs");
        IniWrite.setValue(VarBox.PaperTypes[i][0], VarBox.FamilyNames[i]);
        IniWrite.endGroup();
        qout << "最终图片路径结果：" << str;
        return str;
    }

    QString get_ini_path()
    {
        return get_dat_path() + "\\SpeedBox2.ini";
    }

    QString get_dat_path()
    {
        QString str = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +  "/AppData/Local/SpeedBox");
        get_son_dir(str);
        return str;
    }

    bool check_app_right()
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
                        *ptr = 0; VarBox.AppId = StrJoin(ptr0);
                        ptr += 10; ptr0 = ptr;
                        while (*++ptr && *ptr != '\n');
                        if (*ptr) {
                            *ptr = 0;
                            VarBox.PassWord = StrJoin(ptr0); file.close();
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

    bool build_init_files()
    {
        //getTransCode(TEXT("http://api.fanyi.baidu.com/api/trans/vip/translate?q=English&from=zh&to=en&appid=20210503000812254&salt=1435660288&sign=a1b039c74a3e7b44ad981a1dae445f06", abc);
        QString file = get_ini_path(); short type = 0; qout << "配置文件目录" << file;
        while (true)
        {
            if (type == 0)
            {
                if (!QFile::exists(file))
                {
                    qout << "SpeedBox.ini 文件不存在";
                    sigleSave("SpeedBox", "Version", "21.8.6.1");
                    type = 1;
                    continue;
                }
                else
                {
                    char t[12] = { 0 };
                    GetPrivateProfileString("SpeedBox", "Version", t, t, 11, file.toStdString().c_str());
                    qout << "文件存在，版本信息为" << t;
                    if (StrCompare(t, "21.8.1.0") || StrCompare(t, "21.8.6.1"))
                    {
                        type = 2;
                        continue;
                    }
                    else
                    {
                        qout << "ini文件过期，将其删除。";
                        QFile::remove(file); sigleSave("SpeedBox", "Version", "21.8.6.1");
                        type = 1;
                        continue;
                    }
                }
            }
            if (type == 1)
            {
                qout << "创建新的Ini文件。";
                VarBox.FamilyPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
                VarBox.PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
                VarBox.NativeDir = VarBox.FamilyPath;
                QSettings *IniWrite = new QSettings(get_ini_path(), QSettings::IniFormat);
                IniWrite->beginGroup("Wallpaper");
                IniWrite->setValue("PaperType", (int)VarBox.PaperType);
                IniWrite->setValue("TimeInerval", VarBox.TimeInterval);
                IniWrite->setValue("PageNum", VarBox.PageNum);
                IniWrite->setValue("setNative", false);
                IniWrite->setValue("AutoChange", false);
                IniWrite->setValue("NativeDir", VarBox.FamilyPath);
                IniWrite->setValue("UserCommand", VarBox.UserCommand);
                IniWrite->endGroup();

                IniWrite->beginGroup("Translate");
                IniWrite->setValue("EnableTranslater", false);
                IniWrite->setValue("AutoHide", true);
                IniWrite->endGroup();

                IniWrite->beginGroup("UI");
                IniWrite->setValue("ColorTheme", (int)VarBox.CurTheme);
                IniWrite->setValue("x", 100);
                IniWrite->setValue("y", 100);
                IniWrite->endGroup();

                IniWrite->beginGroup("Dirs");
                IniWrite->setValue("FamilyPath", VarBox.FamilyPath);
                IniWrite->setValue("OpenDir", VarBox.PathToOpen);

                for (short i = 0; i < 10; i++)
                    IniWrite->setValue(VarBox.PaperTypes[i][0], VarBox.FamilyNames[i]);
                IniWrite->endGroup(); delete IniWrite;
                saveTrayStyle();
                type = 3;
                qout << "创建新的ini文件完毕。";
                continue;
            }
            else if (type == 2)
            {
                QSettings *IniRead = new QSettings(get_ini_path(), QSettings::IniFormat);
                IniRead->beginGroup("Wallpaper");
                VarBox.NativeDir = IniRead->value("NativeDir").toString();
                VarBox.PaperType = (PAPER_TYPE)IniRead->value("PaperType").toInt();
                VarBox.TimeInterval = IniRead->value("TimeInerval").toInt();
                VarBox.PageNum = IniRead->value("PageNum").toInt();
                VarBox.UserCommand = IniRead->value("UserCommand").toString();
                VarBox.AutoChange = IniRead->value("AutoChange").toBool();
                IniRead->endGroup();

                IniRead->beginGroup("Translate");
                VarBox.AutoHide = IniRead->value("AutoHide").toBool();
                VarBox.EnableTranslater = IniRead->value("EnableTranslater").toBool();
                IniRead->endGroup();

                IniRead->beginGroup("Dirs");
                VarBox.PathToOpen = IniRead->value("OpenDir").toString();
                VarBox.FamilyPath = IniRead->value("FamilyPath").toString();
                for (short i = 0; i < 10; i++)
                {
                    VarBox.FamilyNames[i] = IniRead->value(VarBox.PaperTypes[i][0]).toString();
                }
                IniRead->endGroup();

                IniRead->beginGroup("UI");
                VarBox.CurTheme = (COLOR_THEME)IniRead->value("ColorTheme").toInt();
                IniRead->endGroup(); delete IniRead;
                readTrayStyle();
                type = 3;
                continue;
            }
            else if (type == 3)
            {
                for (short i = 0; i < 7; i++)
                    get_pic_path(i);
                if (!get_son_dir(VarBox.PathToOpen))
                {
                    VarBox.PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
                    sigleSave("Dirs", "OpenDir", VarBox.PathToOpen);
                }
                VarBox.HaveAppRight = check_app_right();
                VarBox.EnableTranslater = VarBox.HaveAppRight && VarBox.EnableTranslater;
                qout << "VarBox.HaveAppRight" << VarBox.HaveAppRight;
                if (VarBox.HaveAppRight){
                    qout << "AppId" << VarBox.AppId;
                    qout << "PassWord" << VarBox.PassWord;
                }
                qout << "VarBox.EnableTranslater" << VarBox.EnableTranslater;
                break;
            }
        }
        return true;
    }

    QString runCommand(QString program, QStringList argument, short line)
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
            return process.readLine();
        }
        else return QString();
    }

    bool isOnline(bool wait)
    {
        qout << "检测网络连接中。";
        DWORD flag;
        for (int c = 1; VarBox.RunApp && (c <= (wait ? 30 : 1)); c++)
        {
            if (VarBox.RunApp && InternetGetConnectedState(&flag, 0))
            {
                qout << "网络连接正常。";
                return true;
            }
            else if (VarBox.RunApp)
                for (short j = 1; VarBox.RunApp && (j <= 6); j++)
                    Sleep(500);
            qout << "再次检测网络连接。";
        }
        qout << "没有网络连接。";
        return false;
    }

    BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor)    //设置窗口WIN10风格
    {
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
        HMODULE hUser = GetModuleHandle(TEXT("user32.dll"));
        if (hUser)
            pSetWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
        if (pSetWindowCompositionAttribute)
        {
            ACCENT_POLICY accent = { mode, 2, AlphaColor, 0 };
            _WINDOWCOMPOSITIONATTRIBDATA data;
            data.Attrib = WCA_ACCENT_POLICY;
            data.pvData = &accent;
            data.cbData = sizeof(accent);
            ret = pSetWindowCompositionAttribute(hWnd, &data);
        }
        return ret;
    }

    void readTrayStyle()
    {
        QSettings IniRead(get_ini_path(), QSettings::IniFormat);
        IniRead.beginGroup("TaskStyle");
        VarBox.aMode[0] = (_ACCENT_STATE)(IniRead.value("aMode_f").toUInt());
        VarBox.aMode[1] = (_ACCENT_STATE)(IniRead.value("aMode_s").toUInt());
        VarBox.dAlphaColor[0] = IniRead.value("dAlphaColor_f").toULongLong();
        VarBox.dAlphaColor[1] = IniRead.value("dAlphaColor_s").toULongLong();
        VarBox.bAlpha[0] = IniRead.value("bAlpha_f").toUInt();
        VarBox.bAlpha[1] = IniRead.value("bAlpha_s").toUInt();
        VarBox.RefreshTime = IniRead.value("RefreshTime").toInt();
        VarBox.iPos = (TaskBarCenterState)IniRead.value("iPos").toInt();
        IniRead.endGroup();
    }

    void saveTrayStyle()
    {
        QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup("TaskStyle");
        IniWrite.setValue("aMode_f", (int)VarBox.aMode[0]);
        IniWrite.setValue("aMode_s", (int)VarBox.aMode[1]);
        IniWrite.setValue("dAlphaColor_f", (unsigned long long)VarBox.dAlphaColor[0]);
        IniWrite.setValue("dAlphaColor_s", (unsigned long long)VarBox.dAlphaColor[1]);
        IniWrite.setValue("bAlpha_f", (int)VarBox.bAlpha[0]);
        IniWrite.setValue("bAlpha_s", (int)VarBox.bAlpha[1]);
        IniWrite.setValue("RefreshTime", VarBox.RefreshTime);
        IniWrite.setValue("iPos", (int)VarBox.iPos);
        IniWrite.endGroup();
    }

    INT getValue(const char key[], INT dft)
    {
        HKEY pKey; INT value = dft;
        RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_ALL_ACCESS, &pKey);
        if (pKey)
        {
            DWORD dType = REG_DWORD;
            DWORD cbData = sizeof(int);
            RegQueryValueEx(pKey, key, 0, &dType, (BYTE*)&value, &cbData);
            RegCloseKey(pKey);
        }
        return value;
    }

    BOOL delKey(const char key[])
    {
        HKEY pKey;
        RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_ALL_ACCESS, &pKey);
        if (pKey)
        {
            DWORD dType = REG_DWORD;
            DWORD cbData = sizeof(int);
            int enable_sec;
            if ((RegQueryValueEx(pKey, key, 0, &dType, (BYTE*)&enable_sec, &cbData) == ERROR_SUCCESS)
                &&
                (RegDeleteValue(pKey, key) == ERROR_SUCCESS))
            {
                RegCloseKey(pKey);
                return true;
            }
            else
                RegCloseKey(pKey);
        }
        return false;
    }

    void setKey(const char key[], BOOL value)
    {
        HKEY pKey;
        RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_ALL_ACCESS, &pKey);
        if (pKey)
        {
            RegSetValueEx(pKey, key, 0, REG_DWORD, (BYTE*)&value, sizeof(value));
            RegCloseKey(pKey);
        }
    }

    //  检测 TASK_DESK_SUB 下是否存在某值
    BOOL checkKey(const char key[])
    {
        HKEY pKey;
        RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_READ, &pKey);
        if (pKey)
        {
            DWORD dType = REG_DWORD;
            DWORD cbData = sizeof(BOOL);
            BOOL enable_sec;
            if (RegQueryValueEx(pKey, key, 0, &dType, (BYTE*)&enable_sec, &cbData) == ERROR_SUCCESS)
            {
                RegCloseKey(pKey);
                return TRUE;
            }
            else
                RegCloseKey(pKey);
        }
        return FALSE;
    }

    HINSTANCE pShellExecute(_In_opt_ HWND hwnd, _In_opt_ LPCTSTR lpOperation, _In_ LPCTSTR lpFile, _In_opt_ LPCTSTR lpParameters, _In_opt_ LPCTSTR lpDirectory, _In_ INT nShowCmd)
    {
        HINSTANCE hInstance = NULL;
        typedef HINSTANCE(WINAPI* pfnShellExecute)(_In_opt_ HWND hwnd, _In_opt_ LPCTSTR lpOperation, _In_ LPCTSTR lpFile, _In_opt_ LPCTSTR lpParameters, _In_opt_ LPCTSTR lpDirectory, _In_ INT nShowCmd);
        HMODULE hShell32 = LoadLibrary(TEXT("shell32.dll"));
        if (hShell32)
        {
            pfnShellExecute pShellExecuteW = (pfnShellExecute)GetProcAddress(hShell32, "ShellExecuteA");
            if (pShellExecuteW)
                hInstance = pShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
            FreeLibrary(hShell32);
        }
        return hInstance;
    }
}
