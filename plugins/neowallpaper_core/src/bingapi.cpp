#include <bingapi.h>
#include <httplib.h>
#include <iostream>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
#include <sstream>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;
namespace chrono = std::chrono;
using namespace std::literals;

static std::mutex s_AutoDownloadMutex;

BingApi::BingApi(YJson& setting)
  : WallBase(InitSetting(setting))
  , m_Data(nullptr)
{
  InitData();
  AutoDownload();
}

BingApi::~BingApi()
{
  m_QuitFlag = true;
  Locker locker(s_AutoDownloadMutex);
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
  if (fs::exists(m_DataPath) && fs::file_size(m_DataPath) != 0) {
    m_Data = new YJson(m_DataPath, YJson::UTF8);
  }
}

bool BingApi::CheckData()
{
  if (!HttpLib::IsOnline())
    return false;
  // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

  LockerEx locker(m_DataMutex);
  if (m_Setting[u8"curday"].getValueString() != GetToday()) {
    delete m_Data;
    m_Data = nullptr;
  }

  if (m_Data) return true;

  const auto url = m_Setting[u8"api"].getValueString() + u8"/HPImageArchive.aspx?format=js&idx=0&n=8&mkt="s + m_Setting[u8"region"].getValueString();

  locker.unlock();
  HttpLib clt(url);
  auto res = clt.Get();
  if (res->status != 200)
    return false;

  locker.lock();
  if (m_Data) return true;
  m_Data = new YJson(res->body.begin(), res->body.end());

  m_Setting[u8"curday"] = GetToday();
  SaveSetting();
  m_Data->toFile(m_DataPath);
  return true;
}

ImageInfoEx BingApi::GetNext() {
  static size_t s_uCurImgIndex = 0;

  // https://www.bing.com/th?id=OHR.Yellowstone150_ZH-CN055
  // 下这个接口含义，直接看后面的请求参数1084440_UHD.jpg

  ImageInfoEx ptr(new ImageInfo);

  if (!CheckData()) {
    ptr->ErrorMsg = u8"Bad network connection.";
    ptr->ErrorCode = ImageInfo::NetErr;
    return ptr;
  }

  Locker locker(m_DataMutex);
  const fs::path imgDir = m_Setting[u8"directory"].getValueString();
  if (!fs::exists(imgDir) && !fs::create_directories(imgDir)) {
    ptr->ErrorMsg = u8"Can not creat the image directory.";
    ptr->ErrorCode = ImageInfo::FileErr;
    return ptr;
  }

  auto& imgInfo = m_Data->find(u8"images")->second[s_uCurImgIndex];
  m_Setting[u8"copyrightlink"] = imgInfo[u8"copyrightlink"];
  ptr->ImagePath = (imgDir / GetImageName(imgInfo)).u8string();
  ptr->ImageUrl = m_Setting[u8"api"].getValueString() + imgInfo[u8"urlbase"].getValueString() + u8"_UHD.jpg";
  SaveSetting();

  ++s_uCurImgIndex &= 0x07;
  ptr->ErrorCode = ImageInfo::NoErr;
  return ptr;
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
  if (m_Setting[u8"auto-download"sv].isFalse())
    return;
  std::thread([this](){
    Locker _(s_AutoDownloadMutex);

    for (int i=0; i!=300; i++) {
      std::this_thread::sleep_for(100ms);
      if (m_QuitFlag) return;
    }

    LockerEx locker(m_DataMutex);
    if (m_Setting[u8"auto-download"sv].isFalse()) {
      return;
    }
    locker.unlock();

    if (!CheckData()) {
      return;
    }
    
    locker.lock();
    const fs::path imgDir = m_Setting[u8"directory"].getValueString();
    for (auto& item: m_Data->find(u8"images")->second.getArray()) {
      ImageInfoEx ptr(new ImageInfo);
      ptr->ImagePath = (imgDir / GetImageName(item)).u8string();
      ptr->ImageUrl = m_Setting[u8"api"].getValueString() +
        item[u8"urlbase"].getValueString() + u8"_UHD.jpg";
      ptr->ErrorCode = ImageInfo::NoErr;
      locker.unlock();
      Wallpaper::DownloadImage(ptr);
      locker.lock();
    }
  }).detach();
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
