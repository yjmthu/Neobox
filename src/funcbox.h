#ifndef FUNCBOX_H
#define FUNCBOX_H

#ifndef qout
#define qout qDebug()
#endif

#include <windows.h>
#include <iphlpapi.h>
#include <QString>
#include <QDebug>

#define RETCODE_ERROR_EXIT            1071       //异常退出常数
#define RETCODE_UPDATE                1072       //更新常数，更新软件
#define RETCODE_RESTART               1073       //重启常数，双击时界面会返回这个常数实现整个程序重新启动。
#define MSG_APPBAR_MSGID              2731

#define SHOW_SECONDS_IN_SYSTEM_CLOCK "ShowSecondsInSystemClock"
#define TASKBAR_ACRYLIC_OPACITY      "TaskbarAcrylicOpacity"
#define TASK_DESK_SUB                "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"


enum class PAPER_TYPE
{
    Latest = 0, Hot = 1, Nature = 2, Anime = 3, Simple = 4, Random = 5, Bing = 6, Wallpapers = 7, Native = 8, Advance = 9
};

enum class COLOR_THEME
{
    White = 0, Gray = 1, Purple = 2, Red = 3, Green = 4, Blue = 5, Brown = 6, Black = 7
};

enum class WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_LAST = 23
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

enum class ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5,
    ACCENT_ENABLE_TRANSPARENT = 6,
    ACCENT_NORMAL = 150
};

struct ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

enum class TaskBarCenterState
{
    TASK_LEFT = 0,
    TASK_CENTER = 1,
    TASK_RIGHT = 2
};

class Form; struct IAccessible; //void* PMIB_IFTABLE;
class DesktopMask;
typedef ULONG(WINAPI* pfnGetAdaptersAddresses)(_In_ ULONG Family, _In_ ULONG Flags, _Reserved_ PVOID Reserved, _Out_writes_bytes_opt_(*SizePointer) void* AdapterAddresses, _Inout_ PULONG SizePointer);
typedef DWORD(WINAPI* pfnGetIfTable)(_Out_writes_bytes_opt_(*pdwSize) PMIB_IFTABLE pIfTable, _Inout_ PULONG pdwSize, _In_ BOOL bOrder);

typedef ULONG(WINAPI* pfnAccessibleObjectFromWindow)(_In_ HWND hwnd, _In_ DWORD dwId, _In_ REFIID riid, _Outptr_ void** ppvObject);
typedef ULONG(WINAPI* pfnAccessibleChildren)(_In_ IAccessible* paccContainer, _In_ LONG iChildStart, _In_ LONG cChildren, _Out_writes_(cChildren) VARIANT* rgvarChildren, _Out_ LONG* pcObtained);
typedef BOOL(WINAPI* pfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);


struct VARBOX
{
    const char* const Version = "21.10.1", * const Qt = "6.1.3";
    const unsigned char WinVersion; const bool FirstUse[1] = {false};
    std::list<std::pair<bool, wchar_t*>> PicHistory; std::list<std::pair<bool, wchar_t*>>::const_iterator CurPic;
    const char* const StandardNames[10][2] =     //九种壁纸类型
    {
        {"Latest", "最新壁纸"}, {"Hot", "最热壁纸"}, {"Nature", "风景壁纸"},{"Anime", "动漫壁纸"},
        {"Simple", "极简壁纸"}, {"Random", "随机壁纸"},{"Bing", "必应壁纸"},
        {"MajorName", "桌面壁纸"}, {"Native", "本地壁纸"},{"Advance", "高级壁纸"}
    };
    QString CustomNames[10] =
    {
        "最新壁纸","最热壁纸","风景壁纸","动漫壁纸","极简壁纸",
        "随机壁纸","必应壁纸","桌面壁纸","本地壁纸","高级壁纸"
    };
    QString MajorDir;                 //壁纸文件夹的上一级目录
    PAPER_TYPE PaperType = PAPER_TYPE::Latest;               //当下正在使用的壁纸类型
    COLOR_THEME CurTheme = COLOR_THEME::White;
    bool RunApp = true;                        //app运行状态

    bool AutoChange = false;
    unsigned char PageNum = 1, TimeInterval = 15;
    bool UseDateAsBingName = true, AutoSaveBingPicture = true, AutoRotationBingPicture = true;
    QString NativeDir;                  //当下正在使用的用户本地壁纸文件夹
    QString UserCommand = "python.exe -u X:\\xxxxx.py";                //当下正在使用的用户高级命令
    QString PathToOpen;                //要打开的文件夹

    bool HaveAppRight = false,  EnableTranslater = false, AutoHide = false;;
    char* AppId = nullptr, * PassWord = nullptr;

    bool isMax = false, setMax = false;
    ACCENT_STATE aMode[2] = { ACCENT_STATE::ACCENT_DISABLED, ACCENT_STATE::ACCENT_DISABLED };
    DWORD dAlphaColor[2] = { 0x11111111, 0x11111111 }; /* 背景 */
    DWORD bAlpha[2] = {0xff, 0xff};                    /* 图标 */
    TaskBarCenterState iPos = TaskBarCenterState::TASK_LEFT;
    unsigned short RefreshTime = 33; bool FirstChange = true;

    const int ScreenWidth, ScreenHeight;                  //屏幕宽高
    static HANDLE HMutex; HMODULE hOleacc = NULL, hIphlpapi = NULL, hDwmapi = NULL; Form* const form {};
    DesktopMask *ControlDesktopIcon = nullptr;

    pfnGetIfTable GetIfTable = nullptr;
    pfnGetAdaptersAddresses GetAdaptersAddresses = nullptr;

    pfnAccessibleObjectFromWindow AccessibleObjectFromWindow = nullptr;
    pfnAccessibleChildren AccessibleChildren = nullptr;
    pfnDwmGetWindowAttribute pDwmGetWindowAttribute = nullptr;

    VARBOX(int, int); ~VARBOX();
    void loadFunctions();
    void saveTrayStyle();
    QString get_pic_path(short i);
    QString get_wal_path();
    static QString get_ini_path();
    static QString get_dat_path();
    static bool get_son_dir(QString str);
    static void sigleSave(QString group, QString key, QString value);
    static bool isOnline(bool);                                               //检查是否有网络连接，布尔值代表是否保持检测 30 秒
    static char* runCommand(QString program, QStringList argument, short line = 0);
    static bool getWebCode(const char*, std::string&, bool auto_delete);
    //static bool getBingCode(std::string& code);
    static bool downloadImage(const char*, const QString, bool auto_delete = true);
    static bool getTransCode(const char* url, std::string& outcome);
    static BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor);//设置窗口WIN10风格
    static bool versionBefore(const char* A, const char* B);
    static BOOL PathFileExists(LPCSTR pszPath);
    static BOOL PathFileExists(LPWSTR pszPath);
    static BOOL OneDriveFile(const char* file);
    static BOOL OneDriveFile(const wchar_t*file);
    static void MSG(const char*);

private:
     bool check_app_right();
     void readTrayStyle();
};

extern VARBOX* VarBox;

wchar_t* get_reg_paper();

#endif // FUNCBOX_H
