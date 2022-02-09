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

namespace SystemFunctions
{
    BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor);
}

#endif

#endif // SYSTEMFUNCTIONS_H