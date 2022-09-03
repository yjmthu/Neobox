#include <algorithm>
#include <cstdio>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>

inline std::wstring Utf82WideString(const std::u8string& u8Str) {
  int nWideCount = MultiByteToWideChar(
      CP_UTF8, 0, reinterpret_cast<const char*>(u8Str.data()),
      (int)u8Str.size(), NULL, 0);
  if (nWideCount == 0) {
    return std::wstring();
  }
  std::wstring strResult(nWideCount, 0);
  MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(u8Str.data()),
                      (int)u8Str.size(), strResult.data(), nWideCount);
  return strResult;
}

inline std::u8string Wide2Utf8String(const std::wstring& wStr) {
  //获取所需缓冲区大小
  int nU8Count = WideCharToMultiByte(CP_UTF8, 0, wStr.data(), (int)wStr.size(),
                                     NULL, 0, NULL, NULL);
  if (nU8Count == 0) {
    return std::u8string();
  }
  std::u8string strResult(nU8Count, 0);
  WideCharToMultiByte(CP_UTF8, 0, wStr.data(), (int)wStr.size(),
                      reinterpret_cast<char*>(strResult.data()), nU8Count, NULL,
                      NULL);
  return strResult;
}

inline std::wstring Ansi2WideString(const std::string& ansiStr) {
  int nWideCount = MultiByteToWideChar(CP_ACP, 0, ansiStr.data(),
                                       (int)ansiStr.size(), NULL, 0);
  if (nWideCount == 0) {
    return std::wstring();
  }
  std::wstring strResult(nWideCount, 0);
  MultiByteToWideChar(CP_ACP, 0, ansiStr.data(), (int)ansiStr.size(),
                      strResult.data(), nWideCount);
  return strResult;
}

inline std::string Wide2AnsiString(const std::wstring& wStr) {
  //获取所需缓冲区大小
  int nAnsiCount = WideCharToMultiByte(CP_ACP, 0, wStr.data(), (int)wStr.size(),
                                       NULL, 0, NULL, NULL);
  if (nAnsiCount == 0) {
    return std::string();
  }
  std::string strResult(nAnsiCount, 0);
  WideCharToMultiByte(CP_ACP, 0, wStr.data(), (int)wStr.size(),
                      strResult.data(), nAnsiCount, NULL, NULL);
  return strResult;
}

inline std::string Utf82AnsiString(const std::u8string& u8Str) {
  return Wide2AnsiString(Utf82WideString(u8Str));
}

inline std::u8string Ansi2Utf8String(const std::string& ansiStr) {
  return Wide2Utf8String(Ansi2WideString(ansiStr));
}

template <typename _Ty>
void GetCmdOutput(LPCTSTR cmd, _Ty& result) {
  TCHAR buffer[1024];
  FILE* fp;
  std::basic_stringstream<TCHAR> stream;
#ifdef UNICODE
  if ((fp = _wpopen(cmd, L"r"))) {
    while (std::fgetws(buffer, 1024, fp)) {
#else
  if ((fp = _popen(cmd, "r"))) {
    while (std::fgets(buffer, 1024, fp)) {
#endif
      stream << buffer;
    }
    _pclose(fp);
  }
  std::basic_string<TCHAR> str;
  while (std::getline(stream, str)) {
    result.emplace_back(std::move(str));
  }
}

inline std::wstring GetExeFullPath() {
  TCHAR exeFullPath[MAX_PATH];
  GetModuleFileName(NULL, exeFullPath, MAX_PATH);
  return exeFullPath;
}

std::wstring RegReadString(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName);
bool RegWriteString(HKEY dwSubKey,
                    LPCWSTR pPath,
                    LPCWSTR pKeyName,
                    std::wstring& data);
bool RegRemoveValue(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName);

bool enableBlurBehind(HWND winId, DWORD color, bool enable);

#elif def __linux__

template <typename _Char, typename _Ty>
void GetCmdOutput(const char* cmd, _Ty& result) {
  char buffer[1024];
  FILE* ptr;
  result.emplace_back();
  std::stringstream stream;
  if ((ptr = popen(cmd, "r"))) {
    while (fgets((char*)buffer, 1024, ptr)) {
      stream << buffer;
    }
    pclose(ptr);
  }
  std::string str;
  while (std::getline(stream, str)) {
    result.emplace_back(str.begin(), str.end());
  }
}
#endif
