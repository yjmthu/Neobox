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

// template <typename _Ty>
// void GetCmdOutput(const char* cmd, _Ty& result) {
//   std::array<char, (1<<10)> buffer;
//   FILE* ptr;
//   result.emplace_back();
//   std::stringstream stream;
//   if ((ptr = popen(cmd, "r"))) {
//     while (fgets(buffer.data(), buffer.size(), ptr)) {
//       stream << buffer.data();
//     }
//     pclose(ptr);
//   }
//   std::string str;
//   while (std::getline(stream, str)) {
//     result.emplace_back(str.begin(), str.end());
//   }
// }


#endif

#endif
