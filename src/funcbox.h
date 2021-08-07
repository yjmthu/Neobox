#ifndef FUNCBOX_H
#define FUNCBOX_H

#ifndef qout
#define qout qDebug()
#endif

#include <windows.h>
#include <QString>
#include <QDebug>

#define RETCODE_RESTART               1073       //重启常数，双击时界面会返回这个常数实现整个程序重新启动。
#define MSG_APPBAR_MSGID              2731


typedef enum class _PAPER_TYPES
{
    Latest = 0, Hot = 1, Nature = 2, Anime = 3, Simple = 4, Random = 5, Bing = 6, Wallpapers = 7, Native = 8, Advance = 9
} PAPER_TYPE;

typedef enum class _COLOR_THEME
{
    ClassicWhite = 0, SomeGray = 1, TsinghuaPurple = 2, PekinRed = 3
} COLOR_THEME;

#define TASK_DESK_SUB                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"
#define SHOW_SECONDS_IN_SYSTEM_CLOCK "ShowSecondsInSystemClock"
#define TASKBAR_ACRYLIC_OPACITY      "TaskbarAcrylicOpacity"

typedef enum _WINDOWCOMPOSITIONATTRIB
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
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef enum class _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5,
    ACCENT_ENABLE_TRANSPARENT = 6,
    ACCENT_NORMAL = 150
} ACCENT_STATE;

typedef struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY;

enum class TaskBarCenterState
{
    TASK_LEFT = 0,
    TASK_CENTER = 1,
    TASK_RIGHT = 2
};


typedef struct _VAR_BOX
{
    const char* PaperTypes[10][2];      //九种壁纸类型
    QString FamilyNames[10];
    QString FamilyPath;                 //壁纸文件夹的上一级目录
    PAPER_TYPE PaperType;               //当下正在使用的壁纸类型
    COLOR_THEME CurTheme;
    bool RunApp;                        //app运行状态

    bool AutoChange;                    //当下是否已经启用自动更换壁纸
    bool AutoHide;
    short PageNum;                      //当下使用的壁纸页面（每页120张图片
    short TimeInterval;
    QString NativeDir;                  //当下正在使用的用户本地壁纸文件夹
    QString UserCommand;                //当下正在使用的用户高级命令
    QString PathToOpen;                //要打开的文件夹

    int ScreenWidth;                   //屏幕宽度
    int ScreenHeight;                  //屏幕高度

    bool HaveAppRight,  EnableTranslater;
    char* AppId, * PassWord;

    bool WHEN_FULL; bool SET_FULL;
    ACCENT_STATE aMode[2];
    DWORD dAlphaColor[2]; DWORD bAlpha[2];
    TaskBarCenterState iPos;
    short RefreshTime;

     HANDLE HMutex; HMODULE hOleacc; HMODULE hIphlpapi; void* form;
} VAR_BOX;

extern VAR_BOX VarBox;

namespace FuncBox {

    bool get_son_dir(QString str);
    void sigleSave(QString group, QString key, QString value);
    QString get_ini_path();
    QString get_wal_path();
    QString get_dat_path();
    QString get_pic_path(short);
    bool IsDirExist(const char* csDir);
    bool build_init_files();                                           //创建程序所需文件
    bool isOnline(bool);                                               //检查是否有网络连接，布尔值代表是否保持检测 30 秒
    char* runCommand(QString program, QStringList argument, short line = 0);
    bool getWebCode(const char*, std::string&);
    bool getBingCode(std::string& code);
    bool downloadImage(const char*, const QString, bool auto_delete = true);
    bool getTransCode(const char* url, std::string* outcome);
    BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor);//设置窗口WIN10风格
    void readTrayStyle(); void saveTrayStyle();
    INT getValue(const char key[], INT dft);
    BOOL delKey(const char key[]);
    void setKey(const char key[], BOOL value);
    BOOL checkKey(const char key[]);

//    HINSTANCE pShellExecute(_In_opt_ HWND hwnd, _In_opt_ LPCTSTR lpOperation, _In_ LPCTSTR lpFile, _In_opt_ LPCTSTR lpParameters, _In_opt_ LPCTSTR lpDirectory, _In_ INT nShowCmd);
}

#endif // FUNCBOX_H
