#ifndef SYSTEMFUNCTIONS_H
#define SYSTEMFUNCTIONS_H

#include <QObject>
#ifdef Q_OS_WIN32
#include <Windows.h>

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

enum class SystemVersion {
    WindowsNT4_0,
    Windows95,
    Windows98,
    WindowsMe,
    Windows2k,
    WindowsXP,
    WindowsServer03,
    WindowsServer03R2,
    WindowsVista,
    Windows7,
    WindowsServer08,
    WindowsServer08R2,
    Windows8,
    Windows8_1,
    WindowsServer12,
    Windows10,
    Windows11 = Windows10,
    WindowsUnknown
};

namespace SystemFunctions
{
    BOOL setWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor);
    SystemVersion getWindowsVersion();
}

#endif

#endif // SYSTEMFUNCTIONS_H
