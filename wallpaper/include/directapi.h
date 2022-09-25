#include <httplib.h>

#include <wallbase.h>

class DirectApi : public WallBase {
 public:
  explicit DirectApi() : WallBase() { InitBase(); }
  ~DirectApi() override {}
  bool LoadSetting() override {
    using namespace std::literals;
    if (!std::filesystem::exists(m_SettingPath))
      return false;
    m_Setting = new YJson(m_SettingPath, YJson::UTF8);
    auto& data =
        m_Setting->find(u8"ApiData"sv)
            ->second
            .find(m_Setting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_ApiUrl = data[u8"Url"sv].second.getValueString();
    m_ImageDir = data[u8"Directory"].second.getValueString();
    m_ApiPath = data[u8"Paths"sv]
                    .second[data[u8"CurPath"sv].second.getValueInt()]
                    .getValueString();
    m_ImageNameFormat = data[u8"ImageNameFormat"sv].second.getValueString();
    return true;
  }
  ImageInfoEx GetNext() override {
    return ImageInfoEx(new ImageInfo{(m_ImageDir / GetImageName()).u8string(),
                                     m_ApiUrl + m_ApiPath, u8"OK",
                                     ImageInfo::NoErr});
  }
  bool WriteDefaultSetting() override {
    using namespace std::literals;
    m_ApiUrl = u8"https://source.unsplash.com";
    m_ApiPath = u8"/random/2500x1600";
    m_ImageDir = m_HomePicLocation / u8"随机壁纸";
    m_ImageNameFormat = u8"{0:%Y-%m-%d} {0:%H%M%S}.jpg";
    m_Setting = new YJson(YJson::O{
        {u8"ApiUrl"sv, u8"Unsplash"sv},
        {u8"ApiData"sv,
         YJson::O{{u8"Unsplash"sv,
                   YJson::O{{u8"Url"sv, m_ApiUrl},
                            {u8"CurPath"sv, 0},
                            {u8"Paths"sv, {m_ApiPath}},
                            {u8"Directory"sv, m_ImageDir},
                            {u8"ImageNameFormat"sv, m_ImageNameFormat}}},
                  {u8"Xiaowai"sv,
                   YJson::O{{u8"Url"sv, u8"https://api.ixiaowai.cn"sv},
                            {u8"CurPath"sv, 0},
                            {u8"Paths"sv,
                             {u8"/api/api.php"sv, u8"/mcapi/mcapi.php"sv,
                              u8"/gqapi/gqapi.php"sv}},
                            {u8"Directory"sv, m_HomePicLocation / u8"小歪壁纸"},
                            {u8"ImageNameFormat"sv, m_ImageNameFormat}}}}},
    });
    m_Setting->toFile(m_SettingPath);
    return true;
  }
  void SetCurDir(const std::u8string& sImgPath) override {
    using namespace std::literals;
    auto& data =
        m_Setting->find(u8"ApiData"sv)
            ->second
            .find(m_Setting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_ImageDir = sImgPath;
    data[u8"Directory"].second.setText(sImgPath);
    m_Setting->toFile(m_SettingPath);
  }

  void SetJson(bool update) override {
    using namespace std::literals;
    auto& data =
        m_Setting->find(u8"ApiData"sv)
            ->second
            .find(m_Setting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_ApiUrl = data[u8"Url"sv].second.getValueString();
    m_ImageDir = data[u8"Directory"].second.getValueString();
    m_ApiPath = data[u8"Paths"sv]
                    .second[data[u8"CurPath"sv].second.getValueInt()]
                    .getValueString();
    m_ImageNameFormat = data[u8"ImageNameFormat"sv].second.getValueString();
    m_Setting->toFile(m_SettingPath);
  }

  YJson* GetJson() override { return m_Setting; }

 private:
  std::u8string m_ApiUrl;
  std::u8string m_ApiPath;
  std::u8string m_ImageNameFormat;
  std::u8string GetImageName() {
    using namespace std::chrono;
    auto utc = floor<seconds>(system_clock::now());  // Exactly in seconds.
    const auto time = current_zone()->to_local(utc);
    std::string result = std::vformat(
        std::string(m_ImageNameFormat.begin(), m_ImageNameFormat.end()),
        std::make_format_args(time));
    return std::u8string(result.begin(), result.end());
  }
  YJson* m_Setting;
  const char m_SettingPath[13]{"ApiFile.json"};
};
