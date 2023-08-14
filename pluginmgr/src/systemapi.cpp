#include <neobox/systemapi.h>
#include <stdio.h>
#include <yjson/yjson.h>

#include <iostream>

std::unique_ptr<YJson> m_GlobalSetting;
const char* m_szClobalSettingFile = "Setting.json";

#ifdef _WIN32

struct WINCOMPATTRDATA {
  int nAttribute;
  PVOID pData;
  ULONG ulDataSize;
};

struct ACCENT_POLICY {
  ACCENT_STATE AccentState;
  DWORD AccentFlags;
  DWORD GradientColor;
  DWORD AnimationId;
};

typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

BOOL SetWindowCompositionAttribute(HWND hWnd,
                                   ACCENT_STATE mode,
                                   DWORD GradientColor) {
  BOOL ret = FALSE;
  HMODULE hUser = GetModuleHandle(L"user32.dll");
  if (hUser) {
    pSetWindowCompositionAttribute setWindowCompositionAttribute =
        (pSetWindowCompositionAttribute)GetProcAddress(
            hUser, "SetWindowCompositionAttribute");
    if (setWindowCompositionAttribute) {
      ACCENT_POLICY policy = {mode, 0, GradientColor, 0};
      WINCOMPATTRDATA data{19, &policy, sizeof(ACCENT_POLICY)};
      ret = setWindowCompositionAttribute(hWnd, &data);
    }
  }
  return ret;
}

typedef BOOL(WINAPI* pGetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

BOOL GetWindowCompositionAttribute(HWND hWnd, ACCENT_POLICY* accent[[maybe_unused]]) {
  BOOL ret = FALSE;
  HMODULE hUser = GetModuleHandleW(L"user32.dll");
  if (hUser) {
    pGetWindowCompositionAttribute getWindowCompositionAttribute =
        (pGetWindowCompositionAttribute)GetProcAddress(
            hUser, "GetWindowCompositionAttribute");
    if (getWindowCompositionAttribute) {
      ACCENT_POLICY policy[2];
      WINCOMPATTRDATA data{19, policy, sizeof(ACCENT_POLICY) * 2};
      ret = getWindowCompositionAttribute(hWnd, &data);
    }
  }
  return ret;
}

std::wstring RegReadString(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName) {
  std::wstring result;
  HKEY hKey = nullptr;
  if (ERROR_SUCCESS != RegOpenKeyEx(dwSubKey, pPath, 0, KEY_READ, &hKey)) {
    return result;
  }

  DWORD dwType = REG_SZ;
  DWORD dwDataSize = 0;
  PBYTE pData = nullptr;
  if (ERROR_SUCCESS !=
      RegQueryValueExW(hKey, pKeyName, 0, &dwType, pData, &dwDataSize)) {
    RegCloseKey(hKey);
    return result;
  }

  pData = new BYTE[dwDataSize + sizeof(wchar_t)]{0};
  if (ERROR_SUCCESS !=
      RegQueryValueExW(hKey, pKeyName, 0, &dwType, pData, &dwDataSize)) {
    delete[] pData;
    RegCloseKey(hKey);
    return result;
  }
  result = reinterpret_cast<const wchar_t*>(pData);
  delete[] pData;
  RegCloseKey(hKey);
  return result;
}


DWORD RegReadValue(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName) {
  DWORD result = 0;
  HKEY hKey = nullptr;
  if (ERROR_SUCCESS != RegOpenKeyEx(dwSubKey, pPath, 0, KEY_READ, &hKey)) {
    return result;
  }

  DWORD dwType = REG_DWORD;
  DWORD dwDataSize = sizeof(DWORD);
  if (ERROR_SUCCESS !=
      RegQueryValueExW(hKey, pKeyName, 0, &dwType, (PBYTE)&result, &dwDataSize)) {
    RegCloseKey(hKey);
    return result;
  }
  RegCloseKey(hKey);
  return result;
}

bool RegWriteString(HKEY dwSubKey,
                    LPCWSTR pPath,
                    LPCWSTR pKeyName,
                    std::wstring& data) {
  HKEY hKey = nullptr;
  if (ERROR_SUCCESS != RegOpenKeyEx(dwSubKey, pPath, 0, KEY_WRITE, &hKey)) {
    return false;
  }

  if (ERROR_SUCCESS !=
      RegSetValueExW(hKey, pKeyName, 0, REG_SZ,
                     reinterpret_cast<const BYTE*>(data.data()),
                     (DWORD)data.size() * sizeof(wchar_t))) {
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);
  return true;
}

bool RegRemoveValue(HKEY dwSubKey, LPCWSTR pPath, LPCWSTR pKeyName) {
  HKEY hKey = nullptr;
  if (ERROR_SUCCESS != RegOpenKeyEx(dwSubKey, pPath, 0, KEY_WRITE, &hKey)) {
    return false;
  }

  if (ERROR_SUCCESS != RegDeleteValueW(hKey, pKeyName)) {
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);
  return true;
}

#endif
