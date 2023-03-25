#include <systemapi.h>
#include <neoocr.h>
#include <yjson.h>
#include <httplib.h>
#include <pluginmgr.h>

#include <ranges>
#include <set>
#include <regex>

#include <QByteArray>
#include <QImage>
#include <QBuffer>

#ifdef _WIN32
#include <Unknwn.h>
#include <MemoryBuffer.h>

#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Media.Ocr.h>
// #include <winrt/Windows.Media.Effects.h>
// #include <winrt/Windows.Media.MediaProperties.h>
#include <winrt/windows.Graphics.imaging.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Security.Cryptography.h>

// #include <wrl/client.h>

#include <Windows.h>

#pragma comment(lib, "pathcch")
#pragma comment(lib, "windowsapp.lib")

using Windows::Foundation::IMemoryBufferByteAccess;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Globalization;
using namespace winrt::Windows::Media::Ocr;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Security::Cryptography;
#else

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

namespace fs = std::filesystem;
#endif

NeoOcr::NeoOcr(YJson& settings, std::function<void()> callback)
  : CallBackFunction(callback)
  , m_Settings(settings)
#ifdef __linux__
  , m_TessApi(new tesseract::TessBaseAPI)
  , m_TrainedDataDir(fs::path(settings[u8"TessdataDir"].getValueString()).string())
#endif
{
#ifdef _WIN32
  // init_apartment();
#elif defined(__linux__)
  InitLanguagesList();
#endif
}

NeoOcr::~NeoOcr()
{
#ifdef __linux__
  delete m_TessApi;
#endif
}

static auto OpenImageFile(std::wstring uriImage) {
  // auto const& streamRef = RandomAccessStreamReference::CreateFromFile(StorageFile::GetFileFromPathAsync(uriImage).get());
  auto file = StorageFile::GetFileFromPathAsync(uriImage).get();
  auto stream = file.OpenAsync(FileAccessMode::Read).get();
  auto const& decoder = BitmapDecoder::CreateAsync(stream);
  auto softwareBitmap = decoder.get().GetSoftwareBitmapAsync();
  return softwareBitmap;
}

std::u8string NeoOcr::GetText(const QImage &image)
{
  static auto engine = OcrEngine::TryCreateFromUserProfileLanguages();

  SoftwareBitmap softwareBitmap(
      BitmapPixelFormat::Rgba8,
      image.width(), image.height()
  );

  auto buffer = softwareBitmap.LockBuffer(BitmapBufferAccessMode::Write);
  auto reference = buffer.CreateReference();
  auto access = reference.as<IMemoryBufferByteAccess>();
  unsigned char* pPixelData = nullptr;
  unsigned capacity = 0;
  winrt::check_hresult(access->GetBuffer(&pPixelData, &capacity));
  auto bufferLayout = buffer.GetPlaneDescription(0);

  pPixelData += bufferLayout.StartIndex;
#if 1
  std::copy_n(image.bits(), bufferLayout.Stride * bufferLayout.Height, pPixelData);
#else
  for (int i = 0; i < bufferLayout.Height; i++) {
    std::copy_n(image.scanLine(i), bufferLayout.Width * 4, pPixelData);
    pPixelData += bufferLayout.Stride;
  }
#endif

  reference.Close();
  buffer.Close();

  wchar_t back;
  auto notZh = [](wchar_t c) { return 0x4e00 > c || c > 0x9fa5; };
  std::wstring result;
  auto ocrResult = engine.RecognizeAsync(softwareBitmap).get();
  for (const auto& line : ocrResult.Lines()) {
    back = 0;
    for (const auto& word: line.Words()) {
      auto text = word.Text();
      if (back && (notZh(back) || notZh(text.front()))) {
        result.push_back(L' ');
      }
      back = text.front();
      result += text;
    }
    // if (result.ends_with(L' ')) {
    //   result.back() = L'\n';
    // }
    result.push_back(L'\n');
  }

  return Wide2Utf8String(result);
}

std::vector<std::wstring> NeoOcr::GetLanguages()
{
  auto languages = OcrEngine::AvailableRecognizerLanguages();
  std::vector<std::wstring> result;
  for (const auto& language : languages)
  {
    auto text = language.NativeName();
    result.emplace_back(text.begin(), text.end());
  }

  return result;
}

#ifdef __linux__
void NeoOcr::InitLanguagesList()
{
  m_Languages.clear();

  for (const auto& i: m_Settings[u8"Languages"].getArray()) {
    m_Languages += i.getValueString();
    m_Languages.push_back(u8'+');
  }
  
  if (m_Languages.empty()) {
    return;
  }
//  m_Languages.back() = u8'\0';
}

std::u8string NeoOcr::GetText(Pix *pix)
{
  std::u8string result;
  if (m_Languages.empty()) {
    mgr->ShowMsgbox(u8"error", u8"You should set some language first!");
//    m_TessApi->End();
    return result;
  }
  if (m_TessApi->Init(m_TrainedDataDir.c_str(), reinterpret_cast<const char*>(m_Languages.c_str()))) {
    mgr->ShowMsgbox(u8"error", u8"Could not initialize tesseract.");
    return result;
  }
  m_TessApi->SetImage(pix);
  char* szText = m_TessApi->GetUTF8Text();
  result = reinterpret_cast<const char8_t*>(szText);
  m_TessApi->End();
  delete [] szText;
  return result;
}

void NeoOcr::DownloadFile(const std::u8string& url, const fs::path& path)
{
  HttpLib clt(url);
  clt.Get(path);
}

std::u8string NeoOcr::GetLanguageName(const std::u8string& url)
{
  auto iter = std::find(url.crbegin(), url.crend(), u8'/').base();
  return std::u8string(iter, url.end() - 12);
}

void NeoOcr::AddLanguages(const std::vector<std::u8string> &urls)
{
  auto& langsArray = m_Settings[u8"Languages"].getArray();
  auto langsView = langsArray | std::views::transform([](const YJson& item){ return item.getValueString(); });
  std::set<std::u8string> langsSet(langsView.begin(), langsView.end());
  for (const auto& i: urls) {
    if (!i.ends_with(u8".traineddata")) {
      continue;
    }
    auto name = GetLanguageName(i);
    fs::path path = m_Settings[u8"TessdataDir"].getValueString();
    path /= name + u8".traineddata";
    if (langsSet.find(name) == langsSet.end()) {
      langsSet.insert(name);
    }
    if (i.starts_with(u8"http")) {
      DownloadFile(i, path);
    } else if (fs::exists(i)) {
      fs::copy(i, path);
    }
  }
  langsArray.assign(langsView.begin(), langsView.end());
  CallBackFunction();
  InitLanguagesList();
}

void NeoOcr::RmoveLanguages(const std::vector<std::u8string> &names)
{
  auto& jsLangArray = m_Settings[u8"Languages"];
  for (const auto& i: names) {
    jsLangArray.removeByValA(i);
  }
  CallBackFunction();
  InitLanguagesList();
}

void NeoOcr::SetDataDir(const std::u8string &dirname)
{
  auto goodname = dirname | std::views::transform([](const char8_t i) {
    return i == u8'/' ? u8'\\' : i;
  });
  m_Settings[u8"TessdataDir"].getValueString().assign(goodname.begin(), goodname.end());
  CallBackFunction();
}

static inline bool IsBigDuan() {
  const uint16_t s = 1;
  return *reinterpret_cast<const uint8_t*>(&s);
}

Pix* QImage2Pix(const QImage& qImage) {
  static const bool bIsBigDuan = IsBigDuan();
  if (qImage.isNull())
    return nullptr;
  const int width = qImage.width(), height = qImage.height();
  const int depth = qImage.depth(), bytePerLine = qImage.bytesPerLine();
  PIX* pix = pixCreate(width, height, depth);

  if (qImage.colorCount()) {
    PIXCMAP* map = pixcmapCreate(8);
    if (bIsBigDuan) {  // b g r a
      for (const auto& i : qImage.colorTable()) {
        auto cols = reinterpret_cast<const uchar*>(&i);
        pixcmapAddColor(map, cols[2], cols[1], cols[0]);
      }
    } else {  // a r g b
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
#endif
