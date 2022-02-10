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


namespace SystemFunctions
{
    BOOL setWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor)    //设置窗口WIN10风格
    {
        typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(HWND, struct WINDOWCOMPOSITIONATTRIBDATA*);
        if (mode == ACCENT_STATE::ACCENT_DISABLED)
        {
            SendMessageA(hWnd, WM_THEMECHANGED, 0, 0);
            return TRUE;
        }
        BOOL ret = FALSE;
        HMODULE hUser = LoadLibrary(TEXT("user32.dll"));
        if (!hUser) {
            FreeLibrary(hUser);
            return ret;
        }
        auto pSetWindowCompositionAttribute = reinterpret_cast<pfnSetWindowCompositionAttribute>(GetProcAddress(hUser, "SetWindowCompositionAttribute"));
        if (pSetWindowCompositionAttribute)
        {
            ACCENT_POLICY accent = { mode, 2, AlphaColor, 0 };
            WINDOWCOMPOSITIONATTRIBDATA data;
            data.Attrib = WINDOWCOMPOSITIONATTRIB::WCA_ACCENT_POLICY;
            data.pvData = &accent;
            data.cbData = sizeof(accent);
            ret = pSetWindowCompositionAttribute(hWnd, &data);
        }
        FreeLibrary(hUser);
        return ret;
    }

    SystemVersion getWindowsVersion()
    {
        // 先判断是否为win8.1或win10
        using pfnRtlGetNtVersionNumbers =  void (WINAPI*)(LPDWORD, LPDWORD,LPDWORD);
        HINSTANCE hInst = LoadLibrary(TEXT("ntdll.dll"));
        if (!hInst) return SystemVersion::WindowsUnknown;
        DWORD dwMajor, dwMinor, dwBuildNumber;
        auto RtlGetNtVersionNumbers = reinterpret_cast<pfnRtlGetNtVersionNumbers>(GetProcAddress(hInst, "RtlGetNtVersionNumbers"));
        if (!RtlGetNtVersionNumbers)
        {
            FreeLibrary(hInst);
            return SystemVersion::WindowsUnknown;
        }
        RtlGetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuildNumber);
        FreeLibrary(hInst);
        if (dwMajor == 6 && dwMinor == 3)	//win 8.1
            return SystemVersion::Windows8_1;
        if (dwMajor == 10 && dwMinor == 0)	//win 10
            return SystemVersion::Windows10;

        // 判断win8.1以下的版本
        SYSTEM_INFO info;                // 用SYSTEM_INFO结构判断64位AMD处理器
        GetSystemInfo(&info);            // 调用GetSystemInfo函数填充结构
        OSVERSIONINFOEX os;
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if (GetVersionEx((OSVERSIONINFO *)&os))
        {

            // 下面根据版本信息判断操作系统名称
            switch (os.dwMajorVersion)
            {                        //判断主版本号
            case 4:
                switch (os.dwMinorVersion)
                {                //判断次版本号
                case 0:
                    if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
                        return SystemVersion::WindowsNT4_0;
                    else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                        return SystemVersion::Windows95;
                    else
                        return SystemVersion::WindowsUnknown;
                case 10:
                    return SystemVersion::Windows98;
                case 90:
                    return SystemVersion::WindowsMe;
                default:
                    return SystemVersion::WindowsUnknown;

                }
            case 5:
                switch (os.dwMinorVersion)
                {
                case 0:
                    return SystemVersion::Windows2k;
                case 1:
                    return SystemVersion::WindowsXP;
                case 2:
                    if (os.wProductType == VER_NT_WORKSTATION &&
                        info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                        return SystemVersion::WindowsXP;
                    else if (GetSystemMetrics(SM_SERVERR2) == 0)
                        return SystemVersion::WindowsServer03;
                    else if (GetSystemMetrics(SM_SERVERR2) != 0)
                        return SystemVersion::WindowsServer03R2;
                    else
                        return SystemVersion::WindowsUnknown;
                default:
                    return SystemVersion::WindowsUnknown;
                }
            case 6:
                switch (os.dwMinorVersion)
                {
                case 0:
                    if (os.wProductType == VER_NT_WORKSTATION)
                        return SystemVersion::WindowsVista;
                    else
                        return SystemVersion::WindowsServer08;
                case 1:
                    if (os.wProductType == VER_NT_WORKSTATION)
                        return SystemVersion::Windows7;
                    else
                        return SystemVersion::WindowsServer08R2;
                case 2:
                    if (os.wProductType == VER_NT_WORKSTATION)
                        return SystemVersion::Windows8;
                    else
                        return SystemVersion::WindowsServer12;
                default:
                    return SystemVersion::WindowsUnknown;
                }
            default:
                return SystemVersion::WindowsUnknown;
            }
        }
        else
            return SystemVersion::WindowsUnknown;
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
