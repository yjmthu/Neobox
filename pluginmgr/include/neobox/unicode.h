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

std::wstring Utf82Wide(std::u8string_view u8Str);
std::u8string Wide2Utf8(std::wstring_view wStr);

#ifdef _WIN32

[[deprecated("Avoid using Ansi encoding")]]
std::wstring Ansi2Wide(std::string_view ansiStr);

[[deprecated("Avoid using Ansi encoding")]]
std::string Wide2Ansi(std::wstring_view wStr);

[[deprecated("Avoid using Ansi encoding")]]
inline std::string Utf82Ansi(std::u8string_view u8Str) {
  return Wide2Ansi(Utf82Wide(u8Str));
}

[[deprecated("Avoid using Ansi encoding")]]
inline std::u8string Ansi2Utf8(std::string_view ansiStr) {
  return Wide2Utf8(Ansi2Wide(ansiStr));
}

#else
[[deprecated("Use Utf82Wide instead")]]
inline std::wstring Ansi2Wide(std::string_view ansiStr) {
  return Utf82Wide(std::u8string_view(
    reinterpret_cast<const char8_t*>(ansiStr.data()), ansiStr.size()));
};

[[deprecated("Use Wide2Utf8 instead")]]
inline std::string Wide2Ansi(std::wstring_view wStr) {
  auto res =  Wide2Utf8(wStr);
  return std::string(res.begin(), res.end());
}

[[deprecated("Use Utf8AsAnsi instead")]]
inline std::string Utf82Ansi(std::u8string_view u8Str) {
  return Wide2Ansi(Utf82Wide(u8Str));
}

[[deprecated("Use AnsiAsUtf8 instead")]]
inline std::u8string Ansi2Utf8(std::string_view ansiStr) {
  return std::u8string(ansiStr.begin(), ansiStr.end());
}
#endif

#endif // NEOBOX_UNICODE_H
