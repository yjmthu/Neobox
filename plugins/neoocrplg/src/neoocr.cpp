#include <systemapi.h>
#include <neoocr.h>
#include <yjson.h>
#include <httplib.h>
#include <pluginmgr.h>
#include <ocrconfig.h>

#include <ranges>
#include <set>
#include <regex>
#include <mutex>

#include <QByteArray>
#include <QImage>
#include <QBuffer>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <leptonica/pix_internal.h>

std::unique_ptr<Pix, void(*)(Pix*)> QImage2Pix(const QImage& qImage);
namespace fs = std::filesystem;
static std::mutex s_ThreadMutex;

#ifdef _WIN32
#include <Unknwn.h>
#include <MemoryBuffer.h>

#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Media.Ocr.h>
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
namespace WinOcr = winrt::Windows::Media::Ocr;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Security::Cryptography;
#else

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#endif

NeoOcr::NeoOcr(OcrConfig& settings)
  : m_Settings(settings)
  , m_TessApi(new tesseract::TessBaseAPI)
  , m_TrainedDataDir(fs::path(m_Settings.GetTessdataDir()).string())
{
  InitLanguagesList();
  // winrt::init_apartment();
}

NeoOcr::~NeoOcr()
{
#ifdef __linux__
  delete m_TessApi;
#endif
  std::lock_guard<std::mutex> locker(s_ThreadMutex);
}

static auto OpenImageFile(std::wstring uriImage) {
  // auto const& streamRef = RandomAccessStreamReference::CreateFromFile(StorageFile::GetFileFromPathAsync(uriImage).get());
  auto file = StorageFile::GetFileFromPathAsync(uriImage).get();
  auto stream = file.OpenAsync(FileAccessMode::Read).get();
  auto const& decoder = BitmapDecoder::CreateAsync(stream);
  auto softwareBitmap = decoder.get().GetSoftwareBitmapAsync();
  return softwareBitmap;
}

void SaveSoftwareBitmapToFile(SoftwareBitmap& softwareBitmap)
{
  StorageFolder currentfolder = StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().wstring()).get();
  StorageFile outimagefile = currentfolder.CreateFileAsync(L"NEOOCR.jpg", CreationCollisionOption::ReplaceExisting).get();
  IRandomAccessStream writestream = outimagefile.OpenAsync(FileAccessMode::ReadWrite).get();
  BitmapEncoder encoder = BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId(), writestream).get();
  encoder.SetSoftwareBitmap(softwareBitmap);
  encoder.FlushAsync().get();
  writestream.Close();
}

std::u8string NeoOcr::GetText(QImage image)
{
  const auto engine = static_cast<Engine>(m_Settings.GetOcrEngine());
  if (engine == Engine::Windows) {
    if (image.format() != QImage::Format_RGBA8888) {
      image = image.convertToFormat(QImage::Format_RGBA8888);
    }
    return OcrWindows(image);
  } else if (engine == Engine::Tesseract) {
    return OcrTesseract(image);
  } else {
    return u8"~未知服务器~";
  }
}

std::vector<OcrResult> NeoOcr::GetTextEx(const QImage& image)
{
  std::vector<OcrResult> result;
  if (m_Languages.empty()) {
    mgr->ShowMsgbox(L"error", L"You should set some language first!");
//    m_TessApi->End();
    return result;
  }
  if (m_TessApi->Init(m_TrainedDataDir.c_str(), reinterpret_cast<const char*>(m_Languages.c_str()))) {
    mgr->ShowMsgbox(L"error", L"Could not initialize tesseract.");
    return result;
  }

  auto const pix = QImage2Pix(image);
  m_TessApi->SetImage(pix.get());

  // RIL_TEXTLINE 表示识别文本行
  const auto boxes = m_TessApi->GetComponentImages(tesseract::RIL_WORD, true, NULL, NULL);

  for (int i = 0; i != boxes->n; ++i) {
    auto box = boxaGetBox(boxes, i, L_CLONE);
    m_TessApi->SetRectangle(box->x - 1, box->y - 1, box->w + 2, box->h + 2);
    char* ocrResult = m_TessApi->GetUTF8Text();
    int conf = m_TessApi->MeanTextConf();
    result.push_back(OcrResult {
      .text = reinterpret_cast<char8_t*>(ocrResult),
      .x = box->x,
      .y = box->y,
      .w = box->w,
      .h = box->h,
      .confidence = conf,
    });
    boxDestroy(&box);
    delete[] ocrResult;
  }

  m_TessApi->End();
  return result;
}

std::u8string NeoOcr::OcrWindows(const QImage& image)
{
  std::function engine = WinOcr::OcrEngine::TryCreateFromUserProfileLanguages;
  auto name = m_Settings.GetWinLan();
  if (name != u8"user-Profile") {
    const Language lan(Utf82WideString(name));
    if (WinOcr::OcrEngine::IsLanguageSupported(lan)) {
      engine = std::bind(&WinOcr::OcrEngine::TryCreateFromLanguage, lan);
    }
  }

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

  // SaveSoftwareBitmapToFile(softwareBitmap);

  wchar_t back;
  auto notZh = [](wchar_t c) { return 0x4e00 > c || c > 0x9fa5; };
  std::wstring result;
  auto ocrResult = engine().RecognizeAsync(softwareBitmap).get();
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

std::u8string NeoOcr::OcrTesseract(const QImage& image)
{
  std::u8string result;
  if (m_Languages.empty()) {
    mgr->ShowMsgbox(L"error", L"You should set some language first!");
//    m_TessApi->End();
    return result;
  }
  if (m_TessApi->Init(m_TrainedDataDir.c_str(), reinterpret_cast<const char*>(m_Languages.c_str()))) {
    mgr->ShowMsgbox(L"error", L"Could not initialize tesseract.");
    return result;
  }
  auto const pix = QImage2Pix(image);
  m_TessApi->SetImage(pix.get());
  char* szText = m_TessApi->GetUTF8Text();
  result = reinterpret_cast<const char8_t*>(szText);
  m_TessApi->End();
  delete [] szText;
  return result;
}

std::vector<std::pair<std::wstring, std::wstring>> NeoOcr::GetLanguages()
{
  auto languages = WinOcr::OcrEngine::AvailableRecognizerLanguages();
  std::vector<std::pair<std::wstring, std::wstring>> result = {
    {L"user-Profile", L"用户语言"}
  };
  for (const auto& language : languages) {
    result.emplace_back(decltype(result)::value_type {
      language.LanguageTag(), language.NativeName(),
    });
  }

  return result;
}

void NeoOcr::InitLanguagesList()
{
  m_Languages.clear();

  const auto array = m_Settings.GetLanguages();

  if (array.empty()) {
    return;
  }

  m_Languages = std::accumulate(
    std::next(array.begin()), array.end(),
    array.front().getValueString(),
    [](const std::u8string& a, const YJson& b){
      return a + u8"+" + b.getValueString();
    }
  );
}

void NeoOcr::DownloadFile(std::u8string_view url, const fs::path& path)
{
  HttpLib clt(url);
  clt.Get(path);
}

void NeoOcr::SetDropData(std::queue<std::u8string_view>& data)
{
  auto const folder = fs::path(m_Settings.GetTessdataDir());

  std::vector<std::u8string> urls;
  while (!data.empty()) {
    auto str(data.front());
    data.pop();
    if (str.starts_with(u8"http")) {
      urls.emplace_back(str);
    } else {
      fs::path path = str;
      fs::copy(str, folder / path.filename());
    }
  }
  if (!urls.empty()) {
    std::thread([urls, folder](){
      std::lock_guard<std::mutex> locker(s_ThreadMutex);
      for (auto& url: urls) {
        auto pos = url.rfind(u8'/');
        if (pos == url.npos) {
          continue;
        }
        HttpLib(url).Get(folder / url.substr(pos + 1));
      }
    }).detach();
  }
}

std::u8string NeoOcr::GetLanguageName(const std::u8string& url)
{
  auto iter = std::find(url.crbegin(), url.crend(), u8'/').base();
  return std::u8string(iter, url.end() - 12);
}

void NeoOcr::AddLanguages(const std::vector<std::u8string> &urls)
{
  auto langsArray = m_Settings.GetLanguages();
  auto langsView = langsArray | std::views::transform([](const YJson& item){ return item.getValueString(); });
  std::set<std::u8string> langsSet(langsView.begin(), langsView.end());
  for (const auto& i: urls) {
    if (!i.ends_with(u8".traineddata")) {
      continue;
    }
    auto name = GetLanguageName(i);
    fs::path path = m_Settings.GetTessdataDir();
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
  m_Settings.SetLanguages(langsArray);
  InitLanguagesList();
}

void NeoOcr::RmoveLanguages(const std::vector<std::u8string> &names)
{
  YJson jsLangArray = m_Settings.GetLanguages();
  for (const auto& i: names) {
    jsLangArray.removeByValA(i);
  }
  m_Settings.SetLanguages(std::move(jsLangArray.getArray()));
  InitLanguagesList();
}

void NeoOcr::SetDataDir(const std::u8string &dirname)
{
  fs::path path(dirname);
  path.make_preferred();
  m_Settings.SetTessdataDir(path.u8string());
}

static inline bool IsBigDuan() {
  const uint16_t s = 1;
  return *reinterpret_cast<const uint8_t*>(&s);
}

std::unique_ptr<Pix, void(*)(Pix*)> QImage2Pix(const QImage& qImage) {
  static const bool bIsBigDuan = IsBigDuan();
  if (qImage.isNull())
    return std::unique_ptr<Pix, void(*)(Pix*)>(nullptr, [](Pix* pix){pixDestroy(&pix);});
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
    return std::unique_ptr<Pix, void(*)(Pix*)>(pix, [](Pix* pix){pixDestroy(&pix);});
}
