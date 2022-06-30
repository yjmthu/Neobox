#include <httplib.h>

#include "apiclass.hpp"

namespace WallClass {

class BingApi : public WallBase {
 public:
  explicit BingApi(const std::filesystem::path& picHome)
      : WallBase(picHome),
        m_Mft(u8"zh-CN"),
        m_ImageNameFormat(u8"%s %Y%m%d.jpg"),
        m_ApiUrl("https://global.bing.com"),
        m_Setting(nullptr),
        m_CurImageIndex(0) {
    m_ImageDir = m_HomePicLocation / u8"必应壁纸";
    InitBase();
  }
  virtual ~BingApi() { delete m_Setting; }
  virtual bool LoadSetting() override {
    if (std::filesystem::exists(m_SettingPath) &&
        std::filesystem::file_size(m_SettingPath)) {
      m_Setting = new YJson(m_SettingPath, YJson::UTF8);
      if (m_Setting->find(u8"today")->second.getValueString() ==
          GetToday("%Y%m%d")) {
        m_ImageDir = m_Setting->find(u8"imgdir")->second.getValueString();
        m_ImageNameFormat =
            m_Setting->find(u8"imgfmt")->second.getValueString();
        m_CurImageIndex = m_Setting->find(u8"index")->second.getValueInt();
        m_Mft = m_Setting->find(u8"mkt")->second.getValueString();
        return true;
      } else {
        delete m_Setting;
        m_Setting = nullptr;
        return false;
      }
    }
    return false;
  }
  virtual bool WriteDefaultSetting() override {
    // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

    std::string path("/HPImageArchive.aspx?format=js&idx=0&n=8&mkt=");
    path += std::string(m_Mft.begin(), m_Mft.end());
    httplib::Client clt(m_ApiUrl);
    auto res = clt.Get(path.c_str());
    if (res->status != 200) return false;
    m_Setting = new YJson(res->body.begin(), res->body.end());
    m_Setting->append(GetToday("%Y%m%d"), u8"today");
    m_Setting->append(m_ImageDir, u8"imgdir");
    m_Setting->append(m_ImageNameFormat, u8"imgfmt");
    m_Setting->append(m_Mft, u8"mkt");
    m_Setting->append(static_cast<int>(m_CurImageIndex), u8"index");
    m_Setting->toFile(m_SettingPath);
    return true;
  }
  virtual ImageInfoEx GetNext() override {
    // https://www.bing.com/th?id=OHR.Yellowstone150_ZH-CN055下这个接口含义，直接看后面的请求参数1084440_UHD.jpg

    auto jsTemp = m_Setting->find(u8"images")->second.find(m_CurImageIndex);
    ImageInfoEx ptr(new std::vector<std::u8string>);
    ptr->push_back((m_ImageDir / GetImageName(*jsTemp)).u8string());
    ptr->emplace_back(m_ApiUrl.begin(), m_ApiUrl.end());
    if (++m_CurImageIndex > 7) m_CurImageIndex = 0;
    ptr->push_back(jsTemp->find(u8"urlbase")->second.getValueString() +
                   u8"_UHD.jpg");
    m_Setting->find(u8"index")->second.setValue(
        static_cast<int>(m_CurImageIndex));
    m_Setting->toFile(m_SettingPath);
    return ptr;
  }
  virtual void Dislike(const std::filesystem::path& img) override {
    //
  }
  virtual void SetCurDir(const std::filesystem::path& str) override {
    m_ImageDir = str;
    m_Setting->find(u8"imgdir")->second.setText(str);
    m_Setting->toFile(m_SettingPath);
  }

  virtual void SetJson(const std::u8string& str) override {
    delete m_Setting;
    m_Setting = new YJson(str.begin(), str.end());
    m_Setting->toFile(m_SettingPath);

    // 等待重写
    m_Mft.swap(m_Setting->find(u8"mkt")->second.getValueString());
    WriteDefaultSetting();
    return;
  }

  virtual std::u8string GetJson() const override {
    return m_Setting->toU8String(false);
  }

 private:
  std::u8string m_Mft;
  std::u8string m_ImageNameFormat;
  const std::string m_ApiUrl;
  const char m_SettingPath[13]{"BingApi.json"};
  YJson* m_Setting;
  unsigned m_CurImageIndex;
  std::u8string GetToday(const char* fmt) {
    auto t =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), fmt);
    std::string temp(ss.str());
    return std::u8string(temp.begin(), temp.end());
  }
  std::u8string GetImageName(YJson& imgInfo) {
    std::u8string str(m_ImageNameFormat.begin(), m_ImageNameFormat.end());
    auto pos = str.find(u8"%s");
    if (pos != std::string::npos) {
      std::u8string temp = imgInfo.find(u8"copyright")->second.getValueString();
      temp.erase(temp.find(u8" (© "));
      str.replace(pos, 2, temp);
    }
    auto t = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() -
        std::chrono::hours(24 * m_CurImageIndex));
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), (const char*)str.c_str());
    std::string temp(ss.str());
    return std::u8string(temp.begin(), temp.end());
  }
};
}  // namespace WallClass
