#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdint.h>
#include <array>
#include <string>

class Translate {
 public:
  enum class Lan { AUTO, ZH_CN, ZH_TW, EN_US, JA_JP, FR_LU, RU_RU, MAX };
  Translate();
  ~Translate();
  std::u8string GetResult(const std::u8string& text);
  Lan GetCurFromLanguage();
  Lan GetCurToLanguage();
  void SetToLanguage(Lan language);
  void SetFromLanguage(Lan language);
 private:
  class YJson* m_Data;
  const std::array<std::u8string, static_cast<int>(Lan::MAX)> m_LanMap;
};

#endif  // TRANSLATE_H
