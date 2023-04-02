#ifndef NEOOCR_H
#define NEOOCR_H

#include <functional>
#include <string>
#include <filesystem>

namespace tesseract {
  class TessBaseAPI;
}

class QImage;

struct OcrResult {
  std::u8string text;
  int x, y, w, h;
  int confidence;
};

class NeoOcr {
public:
  enum class Engine { Windows, Tesseract, Paddle, Other };
  NeoOcr(class OcrConfig& settings);
  ~NeoOcr();
  std::u8string GetText(QImage image);
  std::vector<OcrResult> GetTextEx(QImage image);
  static std::vector<std::pair<std::wstring, std::wstring>> GetLanguages();
  void InitLanguagesList();
  void AddLanguages(const std::vector<std::u8string>& urls);
  void RmoveLanguages(const std::vector<std::u8string>& names);
  void SetDataDir(const std::u8string& dirname);
private:
  std::u8string OcrWindows(const QImage& image);
  std::u8string OcrTesseract(const QImage& image);
private:
  OcrConfig& m_Settings;
  std::u8string GetLanguageName(const std::u8string& url);
  void DownloadFile(const std::u8string& url,
      const std::filesystem::path& path);
  tesseract::TessBaseAPI* m_TessApi;
  std::u8string m_Languages;
  std::string m_TrainedDataDir;
};

#endif
