#ifndef FUNCBOX_H
#define FUNCBOX_H

#ifndef qout
#define qout qDebug()
#endif


#include <QString>
#include <QDebug>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <Windows.h>
#elif def Q_OS_LINUX
#include <Windows.h>
#endif

constexpr int RETCODE_ERROR_EXIT =           1071;       //异常退出常数
constexpr int RETCODE_UPDATE     =           1072;       //更新常数，更新软件
constexpr int RETCODE_RESTART    =           1073;       //重启常数，双击时界面会返回这个常数实现整个程序重新启动。
constexpr int MSG_APPBAR_MSGID   =           2731;

inline constexpr int _PX_UNUSED(int a=RETCODE_ERROR_EXIT, int b=RETCODE_UPDATE, int c=RETCODE_RESTART, int d=MSG_APPBAR_MSGID){return a+b+c+d;};

#define SHOW_SECONDS_IN_SYSTEM_CLOCK "ShowSecondsInSystemClock"
#define TASKBAR_ACRYLIC_OPACITY      "TaskbarAcrylicOpacity"
#define TASK_DESK_SUB                "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"


enum class COLOR_THEME
{
    White, Gray, Purple, Red, Green, Blue, Brown, Black
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
    TASK_LEFT,
    TASK_CENTER,
    TASK_RIGHT
};

class Form; class Dialog; class Wallpaper; struct IAccessible; //void* PMIB_IFTABLE;
typedef ULONG(WINAPI* pfnAccessibleObjectFromWindow)(_In_ HWND hwnd, _In_ DWORD dwId, _In_ REFIID riid, _Outptr_ void** ppvObject);
typedef ULONG(WINAPI* pfnAccessibleChildren)(_In_ IAccessible* paccContainer, _In_ LONG iChildStart, _In_ LONG cChildren, _Out_writes_(cChildren) VARIANT* rgvarChildren, _Out_ LONG* pcObtained);
typedef BOOL(WINAPI* pfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
typedef BOOL(WINAPI* pfnInternetGetConnectedState)(LPDWORD, DWORD);

class VARBOX: public QObject
{
    Q_OBJECT

public:
    const char* const Version = "22.1.11";
    const bool FirstUse[1] = { false };
    COLOR_THEME CurTheme = COLOR_THEME::White;

    bool EnableTranslater = false, AutoHide = false;
    bool enableUSBhelper = true;  // enableUSBhelper
    QString PathToOpen;                //右键要打开的文件夹

    const int ScreenWidth, ScreenHeight;                  //屏幕宽高
    const int SysScreenWidth, SysScreenHeight;
    static HANDLE HMutex; HMODULE hOleacc = NULL, hDwmapi = NULL, hWininet=NULL;
    Form* const form {0}; Dialog*const dialog {0};

    pfnAccessibleObjectFromWindow AccessibleObjectFromWindow = nullptr;
    pfnAccessibleChildren AccessibleChildren = nullptr;
    pfnDwmGetWindowAttribute pDwmGetWindowAttribute = nullptr;
    std::function<bool(void)> InternetGetConnectedState = nullptr;
#ifdef SystemParametersInfo
#undef SystemParametersInfo
#endif
#ifdef GetFileAttributes
#undef GetFileAttributes
#endif
    std::function<bool(const wchar_t*)> PathFileExists = nullptr;
    std::function<bool(const wchar_t*)> GetFileAttributes = nullptr;
    VARBOX(int, int); ~VARBOX();
    void loadFunctions();
    Wallpaper* wallpaper;                          //壁纸处理类

    void sigleSave(QString group, QString key, QString value);
    static wchar_t* runCmd(const QString & program, const QStringList& argument, short line);
    BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor);//设置窗口WIN10风格
    uint32_t getVersion(const char* A);
public slots:
    static void MSG(const char* text, const char* title="提示", QMessageBox::StandardButtons buttons=QMessageBox::Ok);
    void openDirectory(const QString& dir);
private:
     friend class Form;
     bool check_app_right();
     void initFile();
     void initChildren();
     void initBehaviors();
};

extern VARBOX* VarBox;
#endif // FUNCBOX_H
