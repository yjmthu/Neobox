#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <optional>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

struct Utf8Array {
  const char8_t* begin;
  const char8_t* end;
};

struct LanPair {
  int f;
  int t;
  LanPair(const class YJson& array);
};

class Translate {
public:
  // enum class Lan { AUTO, ZH_CN, ZH_TW, EN_US, JA_JP, FR_LU, RU_RU, MAX };
  enum Source { Baidu = 0, Youdao = 1, None = 2 } m_Source;
  typedef std::vector<std::vector<std::pair<std::u8string, std::vector<std::u8string>>>> LanguageMap;
  typedef std::function<void(const void*, size_t)> Callback ;

public:
  explicit Translate(class TranslateCfg& setting, Callback&& callback);
  ~Translate();

public:
  template<typename _Utf8Array>
  void GetResult(const _Utf8Array& text) {
    const Utf8Array array { 
      reinterpret_cast<const char8_t*>(text.data()),
      reinterpret_cast<const char8_t*>(text.data()) + text.size() };
    if (m_Source == Youdao) {
      GetResultYoudao(array);
    } else {
      GetResultBaidu(array);
    }
  }
  void SetSource(Source dict);
  inline Source GetSource() const { return m_Source; }
  std::optional<std::pair<int, int>> ReverseLanguage();

public:
  const Callback m_Callback;
  LanPair m_LanPairBaidu, m_LanPairYoudao;
  LanPair* m_LanPair;
  static std::map<std::u8string, std::u8string> m_LangNameMap;
  static const LanguageMap m_LanguageCanFromTo;

private:
  std::unique_ptr<class HttpLib> m_Request;

private:
  void GetResultBaidu(const Utf8Array& text);
  void GetResultYoudao(const Utf8Array& text);
  void FormatYoudaoResult(const class YJson& data);
};

#endif  // TRANSLATE_H
