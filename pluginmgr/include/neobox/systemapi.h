#ifndef SYSTEMAPI_H
#define SYSTEMAPI_H

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <array>


#ifdef _WIN32
#include <tchar.h>
#include <windows.h>

#include "unicode.h"

template <typename _Ty>
void GetCmdOutput(std::wstring cmd, _Ty& result) {
  cmd.push_back(L'\0');
  constexpr auto bufSize = 256;

  SECURITY_ATTRIBUTES sa {};                                              
  HANDLE hPipRead = NULL, hPipWrite = NULL;                                                
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);    
  sa.lpSecurityDescriptor = NULL;    
  sa.bInheritHandle = TRUE;    
  if (!CreatePipe(&hPipRead, &hPipWrite, &sa,0))                             
      return;    

  STARTUPINFO si {};
  PROCESS_INFORMATION pi {};
  si.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&si);
  si.hStdError = hPipWrite;
  si.hStdOutput = hPipWrite;
  si.wShowWindow = SW_HIDE;
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

  if (CreateProcess(NULL, cmd.data(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))             
  {
    char szRecvData[bufSize];
    DWORD dwRecvSize;
    std::stringstream stream;
    std::string str;

    if (NULL != hPipWrite)
    {
      CloseHandle(hPipWrite);
      hPipWrite = NULL;
    }

    while(ReadFile(hPipRead, szRecvData, bufSize, &dwRecvSize, NULL))
    {
      stream.write(szRecvData, dwRecvSize);
    }

    while (std::getline(stream, str)) {
      if (str.ends_with('\r')) str.pop_back();
      result.emplace_back(Ansi2Wide(str));
    }
  }    
    
  if (NULL != hPipRead)
	{
		CloseHandle(hPipRead);
		hPipRead = NULL;
	}

  if (NULL != hPipWrite)
	{
		CloseHandle(hPipWrite);
		hPipWrite = NULL;
	}

  if (NULL != pi.hProcess)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}

	if (NULL != pi.hThread)
	{
		CloseHandle(pi.hThread);
		pi.hThread = NULL;
	}

}

inline std::wstring GetExeFullPath() {
  std::wstring exeFullPath(MAX_PATH, '\0');
  GetModuleFileNameW(NULL, exeFullPath.data(), MAX_PATH);
  exeFullPath.erase(exeFullPath.find(L'\0'));
  return exeFullPath;
}

inline std::wstring GetTempPath() {
  std::wstring tempFullPath(MAX_PATH, '\0');
  ::GetTempPathW(MAX_PATH, tempFullPath.data());
  tempFullPath.erase(tempFullPath.find(L'\0'));
  return tempFullPath;
}

inline std::wstring GetTempFileName() {
  WCHAR lpPathBuffer[MAX_PATH];
  GetTempPathW(MAX_PATH, lpPathBuffer);
  std::wstring lpFileName(MAX_PATH, L'\0');
  GetTempFileNameW(lpPathBuffer, L"neobox", 0, lpFileName.data());
  lpFileName.erase(lpFileName.find(L'\0'));
  return lpFileName;
}

std::wstring RegReadString(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName);
bool RegWriteString(HKEY dwSubKey,
                    LPCWSTR pPath,
                    LPCWSTR pKeyName,
                    std::wstring& data);

bool RegRemoveValue(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName);

DWORD RegReadValue(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName);

enum ACCENT_STATE {
  ACCENT_DISABLED,
  ACCENT_ENABLE_GRADIENT,
  ACCENT_ENABLE_TRANSPARENTGRADIENT,
  ACCENT_ENABLE_BLURBEHIND,
  ACCENT_ENABLE_ACRYLICBLURBEHIND,
  ACCENT_INVALID_STATE
};

BOOL SetWindowCompositionAttribute(HWND hWnd,
                                   ACCENT_STATE mode,
                                   DWORD GradientColor);

#elif defined (__linux__)

#include <codecvt>
#include <locale>

template <typename _Ty>
void GetCmdOutput(const char* cmd, _Ty& result) {
  std::array<char, (1<<10)> buffer;
  FILE* ptr;
  result.emplace_back();
  std::stringstream stream;
  if ((ptr = popen(cmd, "r"))) {
    while (fgets(buffer.data(), buffer.size(), ptr)) {
      stream << buffer.data();
    }
    pclose(ptr);
  }
  std::string str;
  while (std::getline(stream, str)) {
    result.emplace_back(str.begin(), str.end());
  }
}


#endif

#endif
