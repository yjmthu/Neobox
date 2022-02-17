#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <shlobj.h>

#include "yjson.h"
#include "zip.h"

typedef std::conditional<std::is_same<TCHAR, char>::value, std::string, std::wstring>::type MString;

#ifdef UNICODE
#define xout std::wcout
#else
#define xout std::cout
#endif

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

MString ToCurString(const std::string& strUtf8)
{
#ifdef UNICODE
    //UTF-8转unicode
    auto len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
    std::wstring strUnicode(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, &strUnicode.front(), len);
    if (!strUnicode.back()) strUnicode.pop_back();
    return strUnicode;
#else
    //UTF-8转unicode
    int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
    wchar_t * strUnicode = new wchar_t[len];//len = 2
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode, len);

    //unicode转gbk
    len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    std::string strTemp(len, 0);//此时的strTemp是GBK编码
    WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, &strTemp.front(), len, NULL, NULL);
    if (!strTemp.back()) strTemp.pop_back();
    delete[] strUnicode;
    return strTemp;
#endif
}

std::string ToAnsiString(const MString& strCur)
{
#ifdef UNICODE
    auto len = WideCharToMultiByte(CP_ACP, 0, strCur.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strAnsi(len, 0);
    WideCharToMultiByte(CP_ACP, 0, strCur.c_str(), -1, &strAnsi.front(), len, NULL, NULL);
    if (!strAnsi.back()) strAnsi.pop_back();
    return strAnsi;
#else
    return strCur;
#endif
}

bool DeleteJsonDirectory(const MString& m_sParentFolder, YJson* m_pJson)
{
    try {
        auto m_folder = m_pJson->find("Folder");
        auto m_files = m_pJson->find("File");
        auto ptr = m_files->getChild();
        while (ptr) {
            auto temp = m_sParentFolder + TEXT("\\") + ToCurString(ptr->getValueString());
            xout << "Delete File: " << temp << std::endl;
            if (!DeleteFile(temp.c_str()))
            {
                xout << TEXT("Faild to delete file ") << temp << std::endl;
            }
            ptr = ptr->getNext();
        }
        ptr = m_folder->getChild();
        while (ptr)
        {
            auto temp = m_sParentFolder + TEXT("\\") + ToCurString(ptr->getKeyString());
            xout << "Delete folder: " << temp << std::endl;
            if (DeleteJsonDirectory(temp, ptr)) {
                if (!RemoveDirectory(temp.c_str())) {
                    xout << TEXT("Faild to delete folder ") << temp << std::endl;
                    return false;
                }
            } else {
                return false;
            }
            ptr = ptr->getNext();
        }
    } catch (...) {
        xout << TEXT("Json file is incorrect.\n");
        return false;
    }
    return true;
}

int on_extract_entry(const char *filename, void *arg) {
    static int i = 0;
    xout << "Extracted: " << filename << " (" << ++i << " of " << *(int *)arg << std::endl;
    return 0;
}

int main()
{
    try {
        MString m_sAppDataPath(MAX_PATH, 0), m_sProFilePath;
        if (!GetModuleFileName(NULL, &m_sAppDataPath.front(), MAX_PATH)) {
            throw MString(TEXT("获取可执行文件目录出错！\n"));
        }
        // m_sExeFolderPath.erase(std::find(m_sExeFolderPath.rbegin(), m_sExeFolderPath.rend(), '\\').base()-1, m_sExeFolderPath.end());
        // MString m_sAppDataPath(MAX_PATH, 0), m_sProFilePath;
        // if (!GetSpecialFolderPath(NULL, &m_sAppDataPath.front(), CSIDL_LOCAL_APPDATA, FALSE)) {
        //     xout << TEXT("获取AppData目录出错！\n");
        //     break;
        // }
        m_sAppDataPath.erase(std::find(m_sAppDataPath.begin(), m_sAppDataPath.end(), 0), m_sAppDataPath.end());
        m_sAppDataPath += TEXT("\\SpeedBox");
        m_sProFilePath = m_sAppDataPath + TEXT("\\profile.json");
        auto m_hFile = CreateFile(m_sProFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if ( m_hFile == INVALID_HANDLE_VALUE) {
            CloseHandle(m_hFile);
            throw MString(TEXT("打开profile.json文件出错！\n"));
        }

        auto m_dFileSize = GetFileSize(m_hFile, NULL);
        decltype(m_dFileSize) m_dSizeRead;
        std::string m_sBuffer(m_dFileSize, 0);
        ReadFile(m_hFile, &m_sBuffer.front(), m_dFileSize, &m_dSizeRead, NULL);
        CloseHandle(m_hFile);
        YJson js(m_sBuffer);
        MString m_sExeFolderPath { ToCurString(js["path"].getValueString()) };
        const char* m_sType = js["type"].getValueString();
        if (!strcmp(m_sType, "exe")) {
            MString m_sOldExeFile { m_sAppDataPath + TEXT("\\SpeedBox.exe") };
            MString m_sNewExeFile { m_sExeFolderPath + TEXT("\\SpeedBox.exe") };
            if (PathFileExists(m_sOldExeFile.c_str())) {
                if (CopyFile(m_sOldExeFile.c_str(), m_sNewExeFile.c_str(), FALSE))
                {
                    DeleteFile(m_sOldExeFile.c_str());
                } else {
                    throw MString(TEXT("拷贝文件出错！\n"));
                }
            } else {
                throw MString(TEXT("找不到已经下载的更新文件！\n"));
            }
        } else if (!strcmp(m_sType, "zip")) {
            xout << TEXT("The update file's type is zip.\n");
            std::string m_sOldZipFile { ToAnsiString(m_sAppDataPath + TEXT("\\Speed-Box.zip")) };
            std::string m_sToFolder { ToAnsiString(m_sExeFolderPath) };
            DeleteJsonDirectory(m_sExeFolderPath, js.find("binary"));
            int arg = js["count"].getValueInt();
            zip_extract(m_sOldZipFile.c_str(), m_sToFolder.c_str(), on_extract_entry, &arg);
            DeleteFileA(m_sOldZipFile.c_str());
        } else {
            throw MString(TEXT("未知的文件类型！\n"));
        }
        MString m_sNewSpeedBox { m_sExeFolderPath + TEXT("\\SpeedBox.exe") };
        ShellExecute(NULL, NULL, m_sNewSpeedBox.c_str(), NULL, NULL, SW_SHOWNORMAL);
    } catch (const MString& errorStr) {
        MessageBox(NULL, errorStr.c_str(), TEXT("出错"), 0);
    }
    return 0;
}
