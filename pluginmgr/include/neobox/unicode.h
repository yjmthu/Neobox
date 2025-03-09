#ifndef NEOBOX_UNICODE_H
#define NEOBOX_UNICODE_H

#include <string>
#include <locale>
#include <vector>

#ifdef _WIN32
#define UTF8_DEFAULT_LOCALE ".UTF-8"
#else
#define UTF8_DEFAULT_LOCALE "C.UTF-8"
#endif

inline std::string Utf8AsAnsi(std::u8string_view u8Str) {
  return std::string(u8Str.begin(), u8Str.end());
}

inline std::u8string AnsiAsUtf8(std::string_view ansiStr) {
  return std::u8string(ansiStr.begin(), ansiStr.end());
}

inline auto SetLocale(const std::string& locale = UTF8_DEFAULT_LOCALE) {
  auto loc = std::locale(locale);
  auto prev = std::locale::global(loc);

  if (std::locale().name() != loc.name()) {
    throw std::runtime_error("setlocale failed!");
  }

  return prev;
}

std::wstring Utf82Wide(std::u8string_view u8Str);
std::u8string Wide2Utf8(std::wstring_view wStr);
std::vector<std::u8string> GetUtf8Argv();

#ifdef _WIN32
#ifdef _DEBUG
std::wstring Ansi2Wide(std::string_view ansiStr);
std::string Wide2Ansi(std::wstring_view wStr);
std::string Utf82Ansi(std::u8string_view u8Str);
std::u8string Ansi2Utf8(std::string_view ansiStr);
#endif
#endif

#endif // NEOBOX_UNICODE_H
