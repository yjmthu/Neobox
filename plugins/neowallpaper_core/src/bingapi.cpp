#include <bingapi.h>
#include <httplib.h>
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

BingApi::BingApi(YJson& setting)
  : WallBase(InitSetting(setting)),
  m_Data(nullptr)
{
  InitData();
  AutoDownload();
}

BingApi::~BingApi()
{
  delete m_Data;
}

YJson& BingApi::InitSetting(YJson& setting) {
  if (setting.isObject()) {
    return setting;
  }
  auto const initDir = GetStantardDir(u8"必应壁纸");
  setting = YJson {
    YJson::O {
      { u8"api", u8"https://global.bing.com"},
      { u8"curday"sv,    GetToday() },
      { u8"directory"sv,  initDir },
      { u8"name-format"sv, u8"{0:%Y-%m-%d} {1}.jpg" },
      { u8"region"sv, u8"zh-CN" },
      { u8"auto-download"sv, false },
      { u8"index"sv, 0}
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
  if (fs::exists(m_DataPath)) {
    m_Data = new YJson(m_DataPath, YJson::UTF8);
  }
}

bool BingApi::CheckData()
{
  if (!HttpLib::IsOnline())
    return false;
  // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

  if (m_Setting[u8"curday"].getValueString() != GetToday()) {
    delete m_Data;
    m_Data = nullptr;
  }

  if (m_Data) return true;

  const auto url = m_Setting[u8"api"].getValueString() + u8"/HPImageArchive.aspx?format=js&idx=0&n=8&mkt="s + m_Setting[u8"region"].getValueString();

  HttpLib clt(url);
  auto res = clt.Get();
  if (res->status != 200)
    return false;

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

  const fs::path imgDir = m_Setting[u8"directory"].getValueString();
  if (!fs::exists(imgDir) && !fs::create_directories(imgDir)) {
    ptr->ErrorMsg = u8"Can not creat the image directory.";
    ptr->ErrorCode = ImageInfo::FileErr;
    return ptr;
  }

  m_Setting[u8"index"].getValueDouble() = s_uCurImgIndex;
  auto& imgInfo = m_Data->find(u8"images")->second[s_uCurImgIndex];
  ptr->ImagePath = (imgDir / GetImageName(imgInfo)).u8string();
  ptr->ImageUrl = m_Setting[u8"api"].getValueString() + imgInfo[u8"urlbase"].getValueString() + u8"_UHD.jpg";

  ++s_uCurImgIndex &= 0x07;
  ptr->ErrorCode = ImageInfo::NoErr;
  return ptr;
}

fs::path BingApi::GetImageDir() const
{
  return m_Setting[u8"directory"].getValueString();
}

void BingApi::SetCurDir(const std::u8string& str)
{
  m_Setting[u8"directory"] = str;
  SaveSetting();
}

void BingApi::SetJson(bool update)
{
  SaveSetting();

  // 等待重写
  delete m_Data;
  m_Data = nullptr;
  return;
}

void BingApi::AutoDownload() {
  if (m_Setting[u8"auto-download"sv].isFalse())
    return;
  std::thread([this](){
    std::this_thread::sleep_for(30s);
    while (ms_IsWorking || !HttpLib::IsOnline()) {
      std::this_thread::sleep_for(1s);
    }

    ms_IsWorking = true;
    if (!CheckData()) {
      ms_IsWorking = false;
      return;
    }

    const fs::path imgDir = m_Setting[u8"directory"].getValueString();
    for (auto& item: m_Data->find(u8"images")->second.getArray()) {
      ImageInfoEx ptr(new ImageInfo);
      ptr->ImagePath = (imgDir / GetImageName(item)).u8string();
      ptr->ImageUrl = m_Setting[u8"api"].getValueString() +
                      item[u8"urlbase"].getValueString() +
                      u8"_UHD.jpg";
      ptr->ErrorCode = ImageInfo::NoErr;
      Wallpaper::DownloadImage(ptr);
    }
    ms_IsWorking = false;
  }).detach();
}

std::u8string BingApi::GetToday() {
  auto utc = chrono::system_clock::now();
  std::string result =
      std::format("{0:%Y-%m-%d}", chrono::current_zone()->to_local(utc));
  return StringAsUtf8(result);
}

std::u8string BingApi::GetImageName(YJson& imgInfo) {
  // see https://codereview.stackexchange.com/questions/156695/converting-stdchronotime-point-to-from-stdstring
  const std::string fmt(Utf8AsString(m_Setting[u8"name-format"].getValueString()));
  const std::string date = Utf8AsString(imgInfo[u8"enddate"].getValueString());
  const std::u8string& copyright =
      imgInfo[u8"copyright"].getValueString();
  const std::u8string& title = imgInfo[u8"title"].getValueString();

  std::tm tm {};
  std::istringstream(date) >> std::get_time(&tm, "%Y%m%d");
  chrono::system_clock::time_point timePoint  = {};
  timePoint += chrono::seconds(std::mktime(&tm)) + 24h;

  std::string result = std::vformat(fmt, 
    std::make_format_args(timePoint,
      Utf8AsString(title), Utf8AsString(copyright)));
  return StringAsUtf8(result);
}
