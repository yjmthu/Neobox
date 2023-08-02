#include <bingapi.h>
#include <httplib.h>
#include <iostream>
#include <stdexcept>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>
#include <download.h>
#include <neotimer.h>

#include <utility>
#include <numeric>
#include <sstream>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;
namespace chrono = std::chrono;
using namespace std::literals;


BingApi::BingApi(YJson& setting)
  : WallBase(InitSetting(setting))
  , m_Data(nullptr)
  , m_Timer(new NeoTimer)
{
  InitData();
  AutoDownload();
}

BingApi::~BingApi()
{
  m_QuitFlag = true;
  delete m_Timer;
  delete m_Data;
}

YJson& BingApi::InitSetting(YJson& setting) {
  if (setting.isObject()) {
    auto& copyrightlink = setting[u8"copyrightlink"];
    if (!copyrightlink.isString())
      copyrightlink = YJson::String;
    return setting;
  }
  auto const initDir = GetStantardDir(u8"必应壁纸");
  setting = YJson {
    YJson::O {
      { u8"api", u8"https://global.bing.com"},
      { u8"curday"sv,    GetToday() },
      { u8"directory"sv,  initDir },
      { u8"name-format"sv, u8"{3:04d}-{4:02d}-{5:02d} {1}.jpg" },
      { u8"region"sv, u8"zh-CN" },
      { u8"auto-download"sv, false },
      { u8"copyrightlink"sv, YJson::String}
    }
  };
  SaveSetting();
  return setting;
}

void BingApi::InitData()
{
  if (m_Setting[u8"curday"].getValueString() != GetToday()) {
    return;
  }
  try {
    m_Data = new YJson(m_DataPath, YJson::UTF8);
  } catch (std::runtime_error error) {
    std::cerr << error.what() << std::endl;
    delete m_Data;
    m_Data = nullptr;
  }
}

void BingApi::CheckData(CheckCallback cbOK, std::optional<CheckCallback> cbNO)
{
  // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

  m_DataMutex.lock();
  if (m_Setting[u8"curday"].getValueString() != GetToday()) {
    delete m_Data;
    m_Data = nullptr;
  }
  const auto url = m_Setting[u8"api"].getValueString() + u8"/HPImageArchive.aspx?format=js&idx=0&n=8&mkt="s + m_Setting[u8"region"].getValueString();
  m_DataMutex.unlock();

  if (m_Data) {
    return cbOK();
  }

  m_DataRequest = std::make_unique<HttpLib>(url, true);
  
  HttpLib::Callback callback = {
    .m_FinishCallback = [this, cbOK, cbNO](auto msg, auto res) {
      if (msg.empty() && res->status / 100 == 2) {
        m_DataMutex.lock();
        m_Data = new YJson(res->body.begin(), res->body.end());
        m_Data->toFile(m_DataPath);

        m_Setting[u8"curday"] = GetToday();
        SaveSetting();
        m_DataMutex.unlock();
        cbOK();
      } else if (cbNO && res->status != -1) {
        (*cbNO)();
      }
    }
  };

  m_DataRequest->GetAsync(std::move(callback));
}

void BingApi::GetNext(Callback callback) {
  static size_t s_uCurImgIndex = 0;

  // https://www.bing.com/th?id=OHR.Yellowstone150_ZH-CN055
  // 下这个接口含义，直接看后面的请求参数1084440_UHD.jpg

  CheckData([callback, this](){

    Locker locker(m_DataMutex);
    const fs::path imgDir = m_Setting[u8"directory"].getValueString();

    auto& imgInfo = m_Data->find(u8"images")->second[s_uCurImgIndex];
    m_Setting[u8"copyrightlink"] = imgInfo[u8"copyrightlink"];
    SaveSetting();

    ++s_uCurImgIndex &= 0x07;

    callback(ImageInfoEx(new ImageInfo{
      .ImagePath = (imgDir / GetImageName(imgInfo)).u8string(),
      .ImageUrl = m_Setting[u8"api"].getValueString() + imgInfo[u8"urlbase"].getValueString() + u8"_UHD.jpg",
      .ErrorCode = ImageInfo::NoErr
    }));

  }, [callback](){
    callback(ImageInfoEx(new ImageInfo{
      .ErrorMsg = u8"Bad network connection.",
      .ErrorCode = ImageInfo::NetErr
    }));
  });

}

void BingApi::SetJson(const YJson& json)
{
  WallBase::SetJson(json);

  m_DataMutex.lock();
  delete m_Data;
  m_Data = nullptr;
  m_DataMutex.unlock();
  return;
}

void BingApi::AutoDownload() {
  Locker locker(m_DataMutex);
  if (m_Setting[u8"auto-download"sv].isFalse())
    return;

  m_Timer->StartTimer(1min, [this]() {
    LockerEx locker(m_DataMutex);
    if (m_Setting[u8"auto-download"sv].isTrue()) {
      locker.unlock();
      CheckData([this]() {
        LockerEx locker(m_DataMutex);
        const fs::path imgDir = m_Setting[u8"directory"].getValueString();
        for (auto& item : m_Data->find(u8"images")->second.getArray()) {
          ImageInfoEx ptr(new ImageInfo);
          ptr->ImagePath = (imgDir / GetImageName(item)).u8string();
          ptr->ImageUrl = m_Setting[u8"api"].getValueString() +
            item[u8"urlbase"].getValueString() + u8"_UHD.jpg";
          ptr->ErrorCode = ImageInfo::NoErr;
          // ------------------------------------ //
          DownloadJob::DownloadImage(ptr, std::nullopt);
        }
        }, std::nullopt);
    }
    m_Timer->Expire();
  });
}

std::u8string BingApi::GetToday() {
#ifdef _WIN32
  auto utc = chrono::system_clock::now();
  std::string result =
      std::format("{0:%Y-%m-%d}", chrono::current_zone()->to_local(utc));
  return std::u8string(result.begin(), result.end());
#else
  time_t timep;
  time(&timep);

  auto const p = gmtime(&timep);
  std::u8string result(11, 0);
      // std::format("{:4d}-{:2d}-{:2d}", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  sprintf(
    reinterpret_cast<char*>(result.data()), "%04d-%02d-%02d",
    p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  result.pop_back();
  return result;
#endif
}

std::u8string BingApi::GetImageName(YJson& imgInfo) {
  // see https://codereview.stackexchange.com/questions/156695/converting-stdchronotime-point-to-from-stdstring
  const auto fmt = Utf82WideString(m_Setting[u8"name-format"].getValueString());
  const std::string date(Utf8AsString(imgInfo[u8"enddate"].getValueString()));
  const std::u8string& copyright =
      imgInfo[u8"copyright"].getValueString();
  const std::u8string& title = imgInfo[u8"title"].getValueString();

  std::tm tm {};
  std::istringstream(date) >> std::get_time(&tm, "%Y%m%d");
  chrono::system_clock::time_point timePoint  = {};
  timePoint += chrono::seconds(std::mktime(&tm)) + 24h;

#ifdef _WIN32
  std::wstring titleUnicode = Utf82WideString(title);
  for (auto& c: titleUnicode) {
    if ("/\\;:"sv.find(c) != std::wstring::npos) {
      c = '_';
    }
  }
  std::wstring result = std::vformat(fmt, 
    std::make_wformat_args(timePoint,
      titleUnicode, Utf82WideString(copyright)));
#else
  time_t timep = chrono::system_clock::to_time_t(timePoint);
  auto const p = gmtime(&timep);
  std::string result = std::vformat(fmt, 
    std::make_format_args(
      0,
      Utf8AsString(title), 
      Utf8AsString(copyright),
      p->tm_year + 1900,
      p->tm_mon + 1,
      p->tm_mday
    ));
#endif
  return Wide2Utf8String(result);
}
