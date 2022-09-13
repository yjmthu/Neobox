#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdint.h>
#include <vector>
#include <string>
#include <map>

class Translate {
 public:
  // enum class Lan { AUTO, ZH_CN, ZH_TW, EN_US, JA_JP, FR_LU, RU_RU, MAX };
  enum class Dict { Baidu, Youdao, Icba } m_Dict;
  Translate();
  ~Translate();
  std::u8string GetResult(const std::u8string& text);
  void SetDict(Dict dict);
  inline Dict GetDict() const { return m_Dict; }
  inline bool IsUsingBaidu() const { return m_Dict == Dict::Baidu; }
  std::pair<size_t, size_t> m_LanPair;
  std::map<std::u8string, std::u8string> m_LanMap;
  const std::vector<std::pair<std::u8string, std::vector<std::u8string>>> m_LanNamesBaidu;
  const std::vector<std::pair<std::u8string, std::vector<std::u8string>>> m_LanNamesYoudao;
 private:
  std::u8string GetResultBaidu(const std::u8string& text);
  std::u8string GetResultYoudao(const std::u8string& text);
  static void FormatYoudaoResult(std::u8string& result, const class YJson& data);
};

#endif  // TRANSLATE_H
