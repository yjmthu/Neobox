#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
#include <functional>
#include <filesystem>

// export module wallpaper2;

using namespace std::literals;
using namespace std::chrono;

/* export */ class DirectApi : public WallBase {
 public:
  explicit DirectApi() : WallBase(true) { InitBase(); }
  ~DirectApi() override {}
  bool LoadSetting() override {
    if (!std::filesystem::exists(m_szSettingPath))
      return false;
    m_pSetting = new YJson(m_szSettingPath, YJson::UTF8);
    auto& data =
        m_pSetting->find(u8"ApiData"sv)
            ->second
            .find(m_pSetting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_u8strApiUrl = data[u8"Url"sv].second.getValueString();
    m_ImageDir = data[u8"Directory"].second.getValueString();
    m_u8strApiPath = data[u8"Paths"sv]
                    .second[data[u8"CurPath"sv].second.getValueInt()]
                    .getValueString();
    m_u8strImgNameFmt = data[u8"ImageNameFormat"sv].second.getValueString();
    return true;
  }
  ImageInfoEx GetNext() override {
    return ImageInfoEx(new ImageInfo{(m_ImageDir / GetImageName()).u8string(),
                                     m_u8strApiUrl + m_u8strApiPath, u8"OK",
                                     ImageInfo::NoErr});
  }
  bool WriteDefaultSetting() override {
    m_u8strApiUrl = u8"https://source.unsplash.com";
    m_u8strApiPath = u8"/random/2500x1600";
    m_ImageDir = ms_HomePicLocation / u8"随机壁纸";
    m_u8strImgNameFmt = u8"{0:%Y-%m-%d} {0:%H%M%S}.jpg";
    m_pSetting = new YJson(YJson::O{
        {u8"ApiUrl"sv, u8"Unsplash"sv},
        {u8"ApiData"sv,
         YJson::O{{u8"Unsplash"sv,
                   YJson::O{{u8"Url"sv, m_u8strApiUrl},
                            {u8"CurPath"sv, 0},
                            {u8"Paths"sv, {m_u8strApiPath}},
                            {u8"Directory"sv, m_ImageDir},
                            {u8"ImageNameFormat"sv, m_u8strImgNameFmt}}},
                  {u8"Xiaowai"sv,
                   YJson::O{{u8"Url"sv, u8"https://api.ixiaowai.cn"sv},
                            {u8"CurPath"sv, 0},
                            {u8"Paths"sv,
                             {u8"/api/api.php"sv, u8"/mcapi/mcapi.php"sv,
                              u8"/gqapi/gqapi.php"sv}},
                            {u8"Directory"sv, ms_HomePicLocation / u8"小歪壁纸"},
                            {u8"ImageNameFormat"sv, m_u8strImgNameFmt}}}}},
    });
    m_pSetting->toFile(m_szSettingPath);
    return true;
  }
  void SetCurDir(const std::u8string& sImgPath) override {
    auto& data =
        m_pSetting->find(u8"ApiData"sv)
            ->second
            .find(m_pSetting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_ImageDir = sImgPath;
    data[u8"Directory"].second.setText(sImgPath);
    m_pSetting->toFile(m_szSettingPath);
  }

  void SetJson(bool update) override {
    auto& data =
        m_pSetting->find(u8"ApiData"sv)
            ->second
            .find(m_pSetting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    auto& paths = data[u8"Paths"sv].second;
    auto& curPath = data[u8"CurPath"].second;
    if (update && curPath.getValueInt() >= paths.sizeA())
      data[u8"CurPath"].second = 0;
    m_u8strApiUrl = data[u8"Url"sv].second.getValueString();
    m_ImageDir = data[u8"Directory"].second.getValueString();
    m_u8strApiPath = paths.emptyA() ? u8""s : paths[curPath.getValueInt()]
                    .getValueString();
    m_u8strImgNameFmt = data[u8"ImageNameFormat"sv].second.getValueString();
    m_pSetting->toFile(m_szSettingPath);
  }

  YJson* GetJson() override { return m_pSetting; }

 private:
  std::u8string m_u8strApiUrl;
  std::u8string m_u8strApiPath;
  std::u8string m_u8strImgNameFmt;
  std::u8string GetImageName() {
    auto utc = floor<seconds>(system_clock::now());  // Exactly in seconds.
    const auto time = current_zone()->to_local(utc);
    std::string result = std::vformat(
        std::string(m_u8strImgNameFmt.begin(), m_u8strImgNameFmt.end()),
        std::make_format_args(time));
    return std::u8string(result.begin(), result.end());
  }
  YJson* m_pSetting;
  const char m_szSettingPath[13]{"ApiFile.json"};
};
