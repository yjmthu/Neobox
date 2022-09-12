#include <QImage>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <varbox.h>
#include <yjson.h>
#include <ranges>

extern void ShowMessage(const std::u8string& title,
                        const std::u8string& text,
                        int type = 0);

inline bool IsBigDuan()
{
  const uint16_t s = 1;
  return *reinterpret_cast<const uint8_t*>(&s);
}

static PIX* QImage2Pix(const QImage& qImage) {
  static const bool bIsBigDuan = IsBigDuan();
  if (qImage.isNull())
    return nullptr;
  const int width = qImage.width(), height = qImage.height();
  const int depth = qImage.depth(), bytePerLine = qImage.bytesPerLine();
  PIX* pix = pixCreate(width, height, depth);

  if (qImage.colorCount()) {
    PIXCMAP* map = pixcmapCreate(8);
    if (bIsBigDuan) {            // b g r a
      for (const auto& i : qImage.colorTable()) {
        auto cols = reinterpret_cast<const uchar*>(&i);
        pixcmapAddColor(map, cols[2], cols[1], cols[0]);
      }
    } else {                     // a r g b
      for (const auto& i : qImage.colorTable()) {
        auto cols = reinterpret_cast<const uchar*>(&i);
        pixcmapAddColor(map, cols[1], cols[2], cols[3]);
      }
    }
    pixSetColormap(pix, map);
  }

  auto start = pixGetData(pix);
  auto wpld = pixGetWpl(pix);

  switch (qImage.format()) {
    case QImage::Format_Mono:
    case QImage::Format_Indexed8:
    case QImage::Format_RGB888:
      for (int i = 0; i < height; ++i) {
        std::copy_n(qImage.scanLine(i), bytePerLine,
                    reinterpret_cast<uchar*>(start + wpld * i));
      }
      break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
      for (int i = 0; i < height; ++i) {
        auto lines = qImage.scanLine(i);
        l_uint32* lined = start + wpld * i;
        if (bIsBigDuan) {
          for (int j = 0; j < width; ++j, lines += 4) {
            l_uint32 pixel;
            composeRGBPixel(lines[2], lines[1], lines[0], &pixel);
            lined[j] = pixel;
          }
        } else {
          for (int j = 0; j < width; ++j, lines += 4) {
            l_uint32 pixel;
            composeRGBPixel(lines[1], lines[2], lines[3], &pixel);
            lined[j] = pixel;
          }
        }
      }
      break;
    default:
      break;
  }

  return pix;
}

QString QImage2Text(const QImage& qImage) {
  using namespace std::literals;
  QString result;
  char* outText;

  const auto& settings = VarBox::GetSettings(u8"Tools");
  std::string path = std::filesystem::path(
                         settings[u8"Ocr.TessdataDir"].second.getValueString())
                         .string();
  auto&& languagesList = settings[u8"Ocr.Languages"].second.getArray() |
                         std::views::transform([](const YJson& json) {
                           const std::u8string& str = json.getValueString();
                           return std::string(str.begin(), str.end());
                         });
  if (languagesList.empty())
    return result;
  std::string languages;
  for (auto&& i : languagesList) {
    languages += i;
    languages.push_back('+');
  }
  languages.back() = '\0';

  tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
  if (api->Init(path.c_str(), languages.data())) {
    ShowMessage(u8"出错"s, u8"Could not initialize tesseract."s);
    return result;
  }

  Pix* image = QImage2Pix(qImage);
  api->SetImage(image);
  outText = api->GetUTF8Text();
  api->End();
  delete api;
  result = outText;
  delete[] outText;
  pixDestroy(&image);

  return result;
}
