#ifndef BUEATRAY_H
#define BUEATRAY_H

#include <QTimer>
#include "funcbox.h"

#define TASK_DESK_SUB L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"
#define  SHOW_SECONDS_IN_SYSTEM_CLOCK L"ShowSecondsInSystemClock"

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

typedef enum _ACCENT_STATE
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

enum TaskBarCenterState
{
    TASK_LEFT = 0,
    TASK_CENTER = 1,
    TASK_RIGHT = 2
};

typedef struct _TRAYSAVE
{
    BOOL WHEN_FULL;
    BOOL SET_FULL;
    ACCENT_STATE aMode[2];
    DWORD dAlphaColor[2];
    DWORD bAlpha[2];
    short refresh_time;
} TRAYSAVE;

typedef ULONG(WINAPI* pfnAccessibleObjectFromWindow)(_In_ HWND hwnd, _In_ DWORD dwId, _In_ REFIID riid, _Outptr_ void** ppvObject);
typedef ULONG(WINAPI* pfnAccessibleChildren)(_In_ IAccessible* paccContainer, _In_ LONG iChildStart, _In_ LONG cChildren, _Out_writes_(cChildren) VARIANT* rgvarChildren, _Out_ LONG* pcObtained);
typedef BOOL(WINAPI*pfnSetWindowCompositionAttribute)(HWND, struct _WINDOWCOMPOSITIONATTRIBDATA*);
typedef BOOL(WINAPI* pfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
BOOL		SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor);//设置窗口WIN10风格

class BueaTray: public QObject
{
    Q_OBJECT

public:
    explicit BueaTray();
    ~BueaTray();
    void readstyle();
    void savestyle();
    TaskBarCenterState iPos = TASK_LEFT;
    void getValue(const WCHAR key[], int &value);
    bool checkKey(const WCHAR key[]);
    bool delKey(const WCHAR key[]);
    void setKey(const WCHAR key[], BOOL);
    static TRAYSAVE TraySave;

    QTimer *beautifyTask;
    QTimer *centerTask;

private:
    HWND hTray = NULL;                     //系统主任务栏窗口句柄
    HWND hTaskWnd = NULL;                  //系统主任务列表窗口句柄
    HWND hReBarWnd = NULL;                 //系统主任务工具窗口句柄
    HWND hTaskListWnd = NULL;

    void GetShellAllWnd();
    void SetTaskBarPos(HWND, HWND, HWND, HWND, BOOL);

private slots:
    void keepTaskBar();
    void centerTaskBar();
};

#endif
