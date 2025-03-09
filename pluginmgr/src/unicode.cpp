#include <neobox/unicode.h>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>

std::wstring Utf82Wide(std::u8string_view u8Str) {
  if (u8Str.empty()) return {};

  int nWideCount = MultiByteToWideChar(
      CP_UTF8, 0, reinterpret_cast<const char*>(u8Str.data()),
      (int)u8Str.size(), NULL, 0);
  if (nWideCount == 0) {
    throw std::runtime_error("MultiByteToWideChar failed to get buffer size");
  }
  std::wstring strResult(nWideCount, 0);
  MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(u8Str.data()),
                      (int)u8Str.size(), strResult.data(), nWideCount);
  return strResult;
}

std::u8string Wide2Utf8(std::wstring_view wStr) {
  if (wStr.empty()) return {};

  //获取所需缓冲区大小
  int nU8Count = WideCharToMultiByte(CP_UTF8, 0, wStr.data(), (int)wStr.size(),
                                     NULL, 0, NULL, NULL);
  if (nU8Count == 0) {
    throw std::runtime_error("WideCharToMultiByte failed to get buffer size");
  }
  std::u8string strResult(nU8Count, 0);
  WideCharToMultiByte(CP_UTF8, 0, wStr.data(), (int)wStr.size(),
                      reinterpret_cast<char*>(strResult.data()), nU8Count, NULL,
                      NULL);
  return strResult;
}

std::wstring Ansi2Wide(std::string_view ansiStr) {
  if (ansiStr.empty()) return {};

  //获取所需缓冲区大小
  int nWideCount = MultiByteToWideChar(CP_ACP, 0, ansiStr.data(),
                                       (int)ansiStr.size(), NULL, 0);
  if (nWideCount == 0) {
    throw std::runtime_error("MultiByteToWideChar failed to get buffer size");
  }
  std::wstring strResult(nWideCount, 0);
  MultiByteToWideChar(CP_ACP, 0, ansiStr.data(), (int)ansiStr.size(),
                      strResult.data(), nWideCount);
  return strResult;
}

std::string Wide2Ansi(std::wstring_view wStr) {
  if (wStr.empty()) return {};

  //获取所需缓冲区大小
  int nAnsiCount = WideCharToMultiByte(CP_ACP, 0, wStr.data(), (int)wStr.size(),
                                       NULL, 0, NULL, NULL);
  if (nAnsiCount == 0) {
    throw std::runtime_error("WideCharToMultiByte failed to get buffer size");
  }
  std::string strResult(nAnsiCount, 0);
  WideCharToMultiByte(CP_ACP, 0, wStr.data(), (int)wStr.size(),
                      strResult.data(), nAnsiCount, NULL, NULL);
  return strResult;
}

std::string Utf82Ansi(std::u8string_view u8Str) {
  return Wide2Ansi(Utf82Wide(u8Str));
}

std::u8string Ansi2Utf8(std::string_view ansiStr) {
  return Wide2Utf8(Ansi2Wide(ansiStr));
}

std::vector<std::u8string> GetUtf8Argv() {
  int argc;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv == NULL) {
    return {};
  }

  std::vector<std::u8string> args;
  for (int i = 0; i < argc; ++i) {
    args.push_back(Wide2Utf8(argv[i]));
  }

  LocalFree(argv);
  return args;
}
#else
#include <fstream>

// UTF-8 到 std::wstring (UTF-32)
std::u8string Wide2Utf8(std::wstring_view wstr) {
  static_assert(sizeof(wchar_t) == sizeof(char32_t), "wchar_t must be 32-bit");

  std::u8string u8str;
  for (wchar_t wc : wstr) {
    char32_t code_point = static_cast<char32_t>(wc);

    // 验证码点有效性
    if (code_point > 0x10FFFF ||
        (code_point >= 0xD800 && code_point <= 0xDFFF)) {
      throw std::runtime_error("Invalid Unicode code point");
    }

    // 编码为 UTF-8
    if (code_point <= 0x7F) {
      u8str += static_cast<char8_t>(code_point);
    } else if (code_point <= 0x7FF) {
      u8str += static_cast<char8_t>(0xC0 | ((code_point >> 6) & 0x1F));
      u8str += static_cast<char8_t>(0x80 | (code_point & 0x3F));
    } else if (code_point <= 0xFFFF) {
      u8str += static_cast<char8_t>(0xE0 | ((code_point >> 12) & 0x0F));
      u8str += static_cast<char8_t>(0x80 | ((code_point >> 6) & 0x3F));
      u8str += static_cast<char8_t>(0x80 | (code_point & 0x3F));
    } else {
      u8str += static_cast<char8_t>(0xF0 | ((code_point >> 18) & 0x07));
      u8str += static_cast<char8_t>(0x80 | ((code_point >> 12) & 0x3F));
      u8str += static_cast<char8_t>(0x80 | ((code_point >> 6) & 0x3F));
      u8str += static_cast<char8_t>(0x80 | (code_point & 0x3F));
    }
  }
  return u8str;
}

// std::wstring (UTF-32) 到 UTF-8
std::wstring Utf82Wide(std::u8string_view str) {
  static_assert(sizeof(wchar_t) == sizeof(char32_t), "wchar_t must be 32-bit");

  std::wstring wstr;
  for (size_t i = 0; i < str.size();) {
    char8_t c = str[i];
    char32_t code_point = 0;

    // 解析 UTF-8 字节流
    if ((c & 0x80) == 0) { // 1-byte
      code_point = c;
      i += 1;
    } else if ((c & 0xE0) == 0xC0) { // 2-byte
      if (i + 1 >= str.size())
        throw std::runtime_error("Invalid UTF-8");
      code_point = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F);
      i += 2;
    } else if ((c & 0xF0) == 0xE0) { // 3-byte
      if (i + 2 >= str.size())
        throw std::runtime_error("Invalid UTF-8");
      code_point =
          ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
      i += 3;
    } else if ((c & 0xF8) == 0xF0) { // 4-byte → 生成 UTF-32 字符
      if (i + 3 >= str.size())
        throw std::runtime_error("Invalid UTF-8");
      code_point = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) |
                   ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
      i += 4;
    } else {
      throw std::runtime_error("Invalid UTF-8");
    }

    // 验证码点有效性
    if (code_point > 0x10FFFF ||
        (code_point >= 0xD800 && code_point <= 0xDFFF)) {
      throw std::runtime_error("Invalid Unicode code point");
    }
    wstr += static_cast<wchar_t>(code_point);
  }
  return wstr;
}

std::vector<std::u8string> GetUtf8Argv() {
  std::vector<std::u8string> args;

  std::ifstream file("/proc/self/cmdline", std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open /proc/self/cmdline");
  }

  std::string cmdline;
  while (std::getline(file, cmdline, '\0')) {
    args.push_back(std::u8string(cmdline.begin(), cmdline.end()));
  }

  file.close();

  return args;
}
#endif
