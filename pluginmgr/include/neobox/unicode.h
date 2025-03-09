#ifndef NEOBOX_UNICODE_H
#define NEOBOX_UNICODE_H

#include <string>
#include <locale>
#include <vector>

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

std::wstring Utf82Wide(std::u8string_view u8Str);
std::u8string Wide2Utf8(std::wstring_view wStr);
std::vector<std::u8string> GetUtf8Argv();

#endif // NEOBOX_UNICODE_H
