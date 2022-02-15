#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>
#include <shlobj.h>
using namespace std;

BOOL PathFileExists(const TCHAR* pszPath)
{
    typedef BOOL(WINAPI* pfnPathFileExists)(const TCHAR* pszPath);
    pfnPathFileExists pPathFileExists = NULL;
    BOOL ret = FALSE;
    HMODULE hShlw = GetModuleHandle(TEXT("Shlwapi.dll"));
    if (hShlw)
#ifdef UNICODE
        pPathFileExists = (pfnPathFileExists)GetProcAddress(hShlw, "PathFileExistsW");
#else
        pPathFileExists = (pfnPathFileExists)GetProcAddress(hShlw, "PathFileExistsA");
#endif
    if (pPathFileExists)
    {
        ret = pPathFileExists(pszPath);
    }
    return ret;
}

typedef BOOL(WINAPI* pfnSHGetSpecialFolderPath)(_Reserved_ HWND hwnd, _Out_writes_(MAX_PATH) LPTSTR pszPath, _In_ int csidl, _In_ BOOL fCreate);
BOOL GetSpecialFolderPath(HWND hwnd,LPTSTR pszPath,int csidl,BOOL fCreate)
{
    pfnSHGetSpecialFolderPath pGetSpecialFolderPath = NULL;
    BOOL ret = FALSE;
    HMODULE hShell = LoadLibrary(TEXT("shell32.dll"));
    if (hShell)
    {
#ifdef UNICODE
        pGetSpecialFolderPath = (pfnSHGetSpecialFolderPath)GetProcAddress(hShell, "SHGetSpecialFolderPathW");
#else
        pGetSpecialFolderPath = (pfnSHGetSpecialFolderPath)GetProcAddress(hShell, "SHGetSpecialFolderPathA");
#endif
        if (pGetSpecialFolderPath)
        {
            ret = pGetSpecialFolderPath(hwnd, pszPath, csidl, fCreate);
        }
        FreeLibrary(hShell);
    }
    return ret;
}

int main()
{
    typedef std::conditional<std::is_same<TCHAR, char>::value, std::string, std::wstring>::type MString;
    MString m_sExeFolderPath(MAX_PATH, 0);
    if (GetModuleFileName(NULL, &m_sExeFolderPath.front(), MAX_PATH))
    {
        m_sExeFolderPath.erase(std::find(m_sExeFolderPath.rbegin(), m_sExeFolderPath.rend(), '\\').base()-1);
    }
    MString m_sAppDataPath(MAX_PATH, 0), m_sProFilePath;
    GetSpecialFolderPath(NULL, &m_sAppDataPath.front(), CSIDL_LOCAL_APPDATA, FALSE);
    m_sAppDataPath.erase(std::find(m_sAppDataPath.begin(), m_sAppDataPath.end(), 0));
    m_sAppDataPath += TEXT("\\SpeedBox");
    m_sProFilePath = m_sAppDataPath + TEXT("\\profile.txt");

    if (PathFileExists(m_sProFilePath.c_str()))
    {
        std::cout << "Hello World!\n";
        // memset(str, 0, sizeof(char) * MAX_PATH);
        // GetModuleFileNameA(NULL, str, MAX_PATH);
        // for (short j = MAX_PATH - 1; j >= 0; --j)
        // {
        //     if (str[j] == '\\')
        //     {
        //         str[j] = 0;
        //         break;
        //     }
        //     str[j] = 0;
        // }
        // string t = str; t += "\\SpeedBox.exe";
        // if (PathFileExists(t.c_str()))
        // {
        //     DeleteFileA(t.c_str());
        // }
        // MoveFileA(s.c_str(), t.c_str());
        // ShellExecuteA(NULL, NULL, t.c_str(), NULL, NULL, SW_SHOWNORMAL);
    } else {
        //
    }
    system("pause");
    return 0;
}
