#ifndef NEOOCR_H
#define NEOOCR_H

#include <functional>
#include <string>
#include <filesystem>

namespace tesseract {
  class TessBaseAPI;
}

class NeoOcr {
public:
  NeoOcr(class YJson& settings, std::function<void()> callback);
  ~NeoOcr();
  std::u8string GetText(class Pix* pix);
  void InitLanguagesList();
  void AddLanguages(const std::vector<std::u8string>& urls);
  void RmoveLanguages(const std::vector<std::u8string>& names);
  void SetDataDir(const std::u8string& dirname);
private:
  std::u8string GetLanguageName(const std::u8string& url);
  void DownloadFile(const std::u8string& url,
      const std::filesystem::path& path);
  const std::function<void()> CallBackFunction;
  class YJson& m_Settings;
  tesseract::TessBaseAPI* m_TessApi;
  std::u8string m_Languages;
  std::string m_TrainedDataDir;
};

#endif
