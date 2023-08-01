#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <optional>
#include <map>
#include <yjson.h>
#include <vector>
#include <functional>
#include <memory>
#include <array>

struct Utf8Array {
  const char8_t* begin;
  const char8_t* end;
};

struct LanPair {
  int f;
  int t;
  LanPair(const YJson::ArrayType& array);
  LanPair(std::array<int, 2> array);
};

class Translate {
public:
  // enum class Lan { AUTO, ZH_CN, ZH_TW, EN_US, JA_JP, FR_LU, RU_RU, MAX };
  enum Source { Baidu, Youdao, BingSimple, None } m_Source;
  typedef std::vector<std::u8string> LanList;
  typedef std::pair<std::u8string, LanList> LanItem;
  typedef std::vector<LanItem> LanMap;
  typedef std::array<LanMap, Source::None> LanMaps;
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
    switch (m_Source) {
    case Baidu:
      GetResultBaidu(array);
      break;
    case Youdao:
      GetResultYoudao(array);
      break;
    case BingSimple:
      GetResultBingSimple(array);
      break;
    default:
      break;
    }
  }
  void SetSource(Source dict);
  inline Source GetSource() const { return m_Source; }
  std::optional<std::pair<int, int>> ReverseLanguage();

public:
  const Callback m_Callback;
  std::array<LanPair, Source::None> m_AllLanPair;
  LanPair* m_LanPair;
  static std::map<std::u8string, std::u8string> m_LangNameMap;
  static const LanMaps m_LanguageCanFromTo;

private:
  std::unique_ptr<class HttpLib> m_Request;

private:
  void GetResultBaidu(const Utf8Array& text);
  void GetResultYoudao(const Utf8Array& text);
  void FormatYoudaoResult(const class YJson& data);
  void GetResultBingSimple(const Utf8Array& text);
};

#endif  // TRANSLATE_H
