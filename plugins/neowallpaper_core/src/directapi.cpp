#include <directapi.h>
#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
#include <functional>
#include <filesystem>

using namespace std::literals;
using namespace std::chrono;

DirectApi::DirectApi(YJson& setting):
  WallBase(InitSetting(setting))
{

}

DirectApi::~DirectApi()
{
  //
}

YJson& DirectApi::InitSetting(YJson& setting)
{
  if (setting.isObject()) {
    return setting;
  }

  setting = YJson::O {
    {u8"ApiUrl"sv, u8"Unsplash"sv},
    {u8"ApiData"sv, YJson::O{
      {u8"Unsplash"sv, YJson::O{
        {u8"Url"sv, u8"https://source.unsplash.com"sv},
        {u8"CurPath"sv, 0},
        {u8"Paths"sv, YJson::A {
          u8"/random/2500x1600"
        }},
        {u8"Directory"sv, GetStantardDir(u8"Unsplash壁纸")},
        {u8"ImageNameFormat"sv, u8"{0:%Y-%m-%d} {0:%H%M%S}.jpg"sv}
      }},
      {u8"Xiaowai"sv, YJson::O{
        {u8"Url"sv, u8"https://api.ixiaowai.cn"sv},
        {u8"CurPath"sv, 0},
        {u8"Paths"sv, YJson::A {
          u8"/api/api.php"sv,
          u8"/mcapi/mcapi.php"sv,
          u8"/gqapi/gqapi.php"sv
        }},
        {u8"Directory"sv, GetStantardDir(u8"小歪壁纸")},
#ifdef _WIN32
        {u8"ImageNameFormat"sv, u8"{0:%Y-%m-%d} {0:%H%M%S}.jpg"sv}
#else
        {u8"ImageNameFormat"sv, u8"{1:04d}-{2:02d}-{3:02d} {4:02d}{5:02d}{6:02d}.jpg"sv}
#endif
      }}
    }},
  };
  SaveSetting();
  return setting;
}
/*
  auto& data =
        m_pSetting->find(u8"ApiData"sv)
            ->second
            .find(m_pSetting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_u8strApiUrl = data[u8"Url"sv].getValueString();
    m_ImageDir = data[u8"Directory"].getValueString();
    m_u8strApiPath = data[u8"Paths"sv][data[u8"CurPath"sv].getValueInt()]
                    .getValueString();
    m_u8strImgNameFmt = data[u8"ImageNameFormat"sv].getValueString();
    return true;
  }
*/

ImageInfoEx DirectApi::GetNext()
{
  Locker locker(m_DataMutex);
  auto& apiInfo = GetCurInfo();
  fs::path const curDir = apiInfo[u8"Directory"].getValueString();
  size_t const curIndex = apiInfo[u8"CurPath"].getValueInt();
  return ImageInfoEx(new ImageInfo
  {
    (curDir / GetImageName()).u8string(),
    apiInfo[u8"Url"].getValueString() + apiInfo[u8"Paths"][curIndex].getValueString(),
    u8"OK",
    ImageInfo::NoErr
  });
}

YJson& DirectApi::GetCurInfo()
{
  return m_Setting[u8"ApiData"][m_Setting[u8"ApiUrl"].getValueString()];
}

// fs::path DirectApi::GetImageDir() const
// {
//   return GetCurInfo()[u8"Directory"].getValueString();
// }

// void DirectApi::SetCurDir(const std::u8string& sImgPath)
// {
//   GetCurInfo()[u8"Directory"] = sImgPath;
//   SaveSetting();
// }

void DirectApi::SetJson(const YJson& json)
{
  WallBase::SetJson(json);

  m_DataMutex.lock();
  auto& apiInfo = GetCurInfo();
  auto& curPaths = apiInfo[u8"Paths"];
  auto& curIndex = apiInfo[u8"CurPath"];
  if (curIndex.getValueInt() >= curPaths.sizeA()) {
    curIndex = 0;
  }
  m_DataMutex.unlock();
}

std::u8string DirectApi::GetImageName() {
  auto& apiInfo = GetCurInfo();

#ifdef _WIN32
  auto utc = floor<seconds>(system_clock::now());  // Exactly in seconds.
  const auto time = current_zone()->to_local(utc);
  const std::string result = std::vformat(
      Utf8AsString(apiInfo[u8"ImageNameFormat"].getValueString()),
      std::make_format_args(time));
#else
  time_t timep;
  time(&timep);

  const auto p = gmtime(&timep);

  const std::string result = std::vformat(
      Utf8AsString(apiInfo[u8"ImageNameFormat"].getValueString()),
      std::make_format_args(
        0,
        p->tm_year + 1900,
        p->tm_mon + 1,
        p->tm_mday,
        p->tm_hour,
        p->tm_min,
        p->tm_sec));
#endif
  return std::u8string(result.begin(), result.end());
}
