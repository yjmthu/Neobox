#include "systemfunctions.h"

#include <QObject>
#ifdef Q_OS_WIN32
#include <Windows.h>

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

struct ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

//typedef ULONG(WINAPI* pfnAccessibleObjectFromWindow)(_In_ HWND hwnd, _In_ DWORD dwId, _In_ REFIID riid, _Outptr_ void** ppvObject);
//typedef ULONG(WINAPI* pfnAccessibleChildren)(_In_ IAccessible* paccContainer, _In_ LONG iChildStart, _In_ LONG cChildren, _Out_writes_(cChildren) VARIANT* rgvarChildren, _Out_ LONG* pcObtained);
//typedef BOOL(WINAPI* pfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
//typedef BOOL(WINAPI* pfnInternetGetConnectedState)(LPDWORD, DWORD);

namespace SystemFunctions
{
    BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor)    //设置窗口WIN10风格
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
}

//#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
//    constexpr DWORD FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS = 0x00400000;
//#endif
//    if (hOleacc)
//    {
//        AccessibleObjectFromWindow = (pfnAccessibleObjectFromWindow)GetProcAddress(hOleacc, "AccessibleObjectFromWindow");
//        AccessibleChildren = (pfnAccessibleChildren)GetProcAddress(hOleacc, "AccessibleChildren");
//    }

//    if (hDwmapi)
//    {
//        pDwmGetWindowAttribute = (pfnDwmGetWindowAttribute)GetProcAddress(hDwmapi, "DwmGetWindowAttribute");
//    }

//    if (hWininet)
//    {
//        static const auto pInternetGetConnectedState = (pfnInternetGetConnectedState)GetProcAddress(hWininet, "InternetGetConnectedState");
//        InternetGetConnectedState = []()->bool{
//            DWORD flag;
//            return pInternetGetConnectedState(&flag, 0);
//        };
//    }


//    if (!(AccessibleObjectFromWindow && AccessibleChildren && pDwmGetWindowAttribute))
//    {
//        qApp->exit(RETCODE_ERROR_EXIT);
//    }
//    PathFileExists = [](const wchar_t* pszPath)->bool{
//        typedef BOOL(WINAPI* pfnPathFileExists)(LPWSTR);
//        pfnPathFileExists pPathFileExists = NULL;
//        BOOL ret = FALSE;
//        HMODULE hUser = GetModuleHandleW(L"Shlwapi.dll");
//        if (hUser)
//            pPathFileExists = (pfnPathFileExists)GetProcAddress(hUser, "PathFileExistsW");
//        if (pPathFileExists)
//        {
//            ret = pPathFileExists((wchar_t*)pszPath);
//        }
//        else
//        {
//        }
//        return ret;
//    };
//    GetFileAttributes = [](const wchar_t* ph)->bool{
//        return GetFileAttributesW(ph) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS;
//    };

#endif
