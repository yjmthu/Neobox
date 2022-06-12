#include "apiclass.hpp"

namespace WallClass {

class DirectApi : public WallBase {
 public:
  explicit DirectApi(const std::filesystem::path& picHome) : WallBase(picHome) {
    InitBase();
  }
  virtual ~DirectApi() {}
  virtual bool LoadSetting() override {
    using namespace std::literals;
    if (!std::filesystem::exists(m_SettingPath)) return false;
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
  virtual ImageInfoEx GetNext() override {
    return ImageInfoEx(new std::vector<std::u8string>{
        (m_ImageDir / GetImageName()).u8string(), m_ApiUrl, m_ApiPath});
  }
  virtual void Dislike(const std::filesystem::path& img) override {
    // remove(img.c_str());
  }
  virtual bool WriteDefaultSetting() override {
    using namespace std::literals;
    m_ApiUrl = u8"https://source.unsplash.com";
    m_ApiPath = u8"/random/2500x1600";
    m_ImageDir = m_HomePicLocation / u8"随机壁纸";
    m_ImageNameFormat = u8"%F %T.jpg";
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
  virtual void SetCurDir(const std::filesystem::path& str) override {
    using namespace std::literals;
    auto& data =
        m_Setting->find(u8"ApiData"sv)
            ->second
            .find(m_Setting->find(u8"ApiUrl"sv)->second.getValueString())
            ->second;
    m_ImageDir = str;
    data[u8"Directory"].second.setText(m_ImageDir.u8string());
    m_Setting->toFile(m_SettingPath);
  }

  virtual const void* GetDataByName(const char* key) const override {
    if (!strcmp(key, "m_Setting")) {
      return &m_Setting;
      // } else if (!strcmp(key, "m_Data")) {
      //     return &m_Data;
      // } else if (!strcmp(key, "m_ImageUrl")) {
      //     return &m_ImageUrl;
    } else {
      return nullptr;
    }
  }

  virtual void Update(bool update) override {
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

 private:
  std::u8string m_ApiUrl;
  std::u8string m_ApiPath;
  std::u8string m_ImageNameFormat;
  std::u8string GetImageName() {
    auto t =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), reinterpret_cast<const char*>(
                                                m_ImageNameFormat.c_str()));
    std::string&& str = ss.str();
    return std::u8string(str.begin(), str.end());
  }
  YJson* m_Setting;
  const char m_SettingPath[13]{"ApiFile.json"};
};
}  // namespace WallClass
