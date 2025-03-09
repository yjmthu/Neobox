#ifndef NEOBOX_UNICODE_H
#define NEOBOX_UNICODE_H

#include <string>
#include <locale>

inline std::string Utf8AsAnsi(std::u8string_view u8Str) {
  return std::string(u8Str.begin(), u8Str.end());
}

inline std::u8string AnsiAsUtf8(std::string_view ansiStr) {
  return std::u8string(ansiStr.begin(), ansiStr.end());
}

inline auto SetLocale(const std::string& locale = "zh_CN.UTF-8") {
  auto loc = std::locale(locale);
  auto prev = std::locale::global(loc);

  if (std::locale().name() != loc.name()) {
    throw std::runtime_error("setlocale failed!");
  }

  return prev;
}

#ifdef _WIN32
#include <Windows.h>

inline std::wstring Utf82Wide(std::u8string_view u8Str) {
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

inline std::u8string Wide2Utf8(std::wstring_view wStr) {
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

inline std::wstring Ansi2Wide(std::string_view ansiStr) {
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

inline std::string Wide2Ansi(std::wstring_view wStr) {
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

inline std::string Utf82Ansi(std::u8string_view u8Str) {
  return Wide2Ansi(Utf82Wide(u8Str));
}

inline std::u8string Ansi2Utf8(std::string_view ansiStr) {
  return Wide2Utf8(Ansi2Wide(ansiStr));
}

#else
inline std::wstring Utf82Wide(std::u8string_view u8Str) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  auto buffer = new char[u8Str.size() + 1] { };
  std::copy(u8Str.begin(), u8Str.end(), buffer);
  std::wstring result = converter.from_bytes(buffer);
  delete[] buffer;

  return result;
}

inline std::u8string Wide2Utf8(std::wstring_view wStr) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  auto result = converter.to_bytes(wStr.data());

  return std::u8string(result.begin(), result.end());
}

inline std::wstring Ansi2Wide(std::string_view ansiStr) {
  return Utf82Wide(std::u8string_view(
    reinterpret_cast<const char8_t*>(ansiStr.data()), ansiStr.size()));
};

inline std::string Wide2Ansi(std::wstring_view wStr) {
  return Wide2Utf8(wStr);
}

inline std::string Utf82Ansi(std::u8string_view u8Str) {
  return Wide2Ansi(Utf82Wide(u8Str));
}

inline std::u8string Ansi2Utf8(std::string_view ansiStr) {
  return std::u8string(ansiStr.begin(), ansiStr.end());
}
#endif

#endif // NEOBOX_UNICODE_H
