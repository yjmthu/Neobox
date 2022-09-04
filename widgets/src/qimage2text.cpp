#include <QImage>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <ranges>
#include <yjson.h>
#include <varbox.h>

static PIX* QImage2Pix(const QImage& qImage) {
  if (qImage.isNull()) return nullptr;
  const int width = qImage.width(), height = qImage.height();
  const int depth = qImage.depth(), bytePerLine = qImage.bytesPerLine();
  PIX *pix = pixCreate(width, height, depth);
  
  if (qImage.colorCount()) {
    PIXCMAP* map = pixcmapCreate(8);
    for (const auto& i: qImage.colorTable()) {
      pixcmapAddColor(map, qRed(i), qGreen(i), qBlue(i));
    }
    pixSetColormap(pix, map);
  }

  auto start = pixGetData(pix);
  auto wpld = pixGetWpl(pix);

  switch (qImage.format()) {
   case QImage::Format_Mono:
   case QImage::Format_Indexed8:
   case QImage::Format_RGB888:
    for (int i=0; i < height; ++i) {
      std::copy_n(qImage.scanLine(i), bytePerLine,
          reinterpret_cast<uchar*>(start + wpld * i));
    }
    break;
   case QImage::Format_RGB32:
   case QImage::Format_ARGB32:
    for (int i=0; i < height; ++i) {
      auto lines = qImage.scanLine(i);
      l_uint32 * lined = start + wpld * i ;
      for(int j = 0; j < width; ++j, lines+=4)
      {
        l_uint32 pixel;
        composeRGBPixel(lines[1], lines[2], lines[3], &pixel);
        lined[j] = pixel;
      }
     }
    break;
   default:
    break;
  }

  return pix;
}

QString QImage2Text(const QImage& qImage)
{
  QString result;
  char *outText;

  auto& settings = VarBox::GetSettings(u8"Tools");
  std::string path = std::filesystem::path(settings[u8"Ocr.TessdataDir"].second.getValueString()).string();
  auto&& languagesList = settings[u8"Ocr.Languages"].second.getArray() |
    std::views::transform([](const YJson& json){
        const std::u8string& str = json.getValueString();
        return std::string(str.begin(), str.end());
      });
  if (languagesList.empty()) return result;
  std::string languages;
  for (auto&& i: languagesList) {
    languages += i;
    languages.push_back('+');
  }
  languages.back() = '\0';

  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  if (api->Init(path.c_str(), languages.data())) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    return result;
  }

  Pix *image = QImage2Pix(qImage);
  api->SetImage(image);
  outText = api->GetUTF8Text();
  api->End();
  delete api;
  result = outText;
  delete [] outText;
  pixDestroy(&image);

  return result;
}
