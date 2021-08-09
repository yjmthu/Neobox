#include <iostream>
#include <windows.h>
#include <shlobj.h>
using namespace std;

typedef BOOL(WINAPI* pfnPathFileExists)(LPCSTR pszPath);
BOOL PathFileExists(LPCSTR pszPath)
{
    pfnPathFileExists pPathFileExists = NULL;
    BOOL ret = FALSE;
    HMODULE hShlw = GetModuleHandle(TEXT("Shlwapi.dll"));
    if (hShlw)
        pPathFileExists = (pfnPathFileExists)GetProcAddress(hShlw, "PathFileExistsA");
    if (pPathFileExists)
    {
        ret = pPathFileExists(pszPath);
    }
    return ret;
}

typedef BOOL(WINAPI*pfnSHGetSpecialFolderPath)(_Reserved_ HWND hwnd, _Out_writes_(MAX_PATH) LPSTR pszPath, _In_ int csidl, _In_ BOOL fCreate);
BOOL GetSpecialFolderPath(_Reserved_ HWND hwnd, _Out_writes_(MAX_PATH) LPSTR pszPath, _In_ int csidl, _In_ BOOL fCreate)
{
    pfnSHGetSpecialFolderPath pGetSpecialFolderPath = NULL;
    BOOL ret = FALSE;
    HMODULE hShell = LoadLibrary(TEXT("shell32.dll"));
    if (hShell)
    {
        pGetSpecialFolderPath = (pfnSHGetSpecialFolderPath)GetProcAddress(hShell, "SHGetSpecialFolderPathA");
        if (pGetSpecialFolderPath)
        {
            ret = pGetSpecialFolderPath(hwnd, pszPath, csidl, fCreate);
        }
        FreeLibrary(hShell);
    }
}

int main()
{
    cout << "请稍等...\n";
    Sleep(1000); char str[MAX_PATH] = { 0 };
    GetSpecialFolderPath(NULL, str, CSIDL_LOCAL_APPDATA, FALSE);
    string s = str;
    s += "\\SpeedBox\\SpeedBox.exe";
    if (PathFileExists(s.c_str()))
    {
        cout << "正在复制文件...\n";
        memset(str, 0, sizeof(char) * MAX_PATH);
		GetModuleFileNameA(NULL, str, MAX_PATH);
        for (short j = MAX_PATH - 1; j >= 0; --j)
        {
            if (str[j] == '\\')
            {
                str[j] = 0;
                break;
            }
            str[j] = 0;
        }
        string t = str; t += "\\SpeedBox.exe";
        if (PathFileExists(t.c_str()))
        {
            DeleteFileA(t.c_str());
        }
        MoveFileA(s.c_str(), t.c_str());
        cout << "复制文件成功！即将启动新版本————" << endl;
        ShellExecuteA(NULL, NULL, t.c_str(), NULL, NULL, SW_SHOWNORMAL);
        cout << "Speed-Box 升级成功！" << endl;
    }
    else
    {
        cout << "未成功下载新版本！\n";
    }
    Sleep(1000);
    return 0;
}
