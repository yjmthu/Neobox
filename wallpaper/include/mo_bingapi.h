#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
// #include <functional>
#include <filesystem>

// export module wallpaper1;

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using namespace std::literals;

/* export */ class BingApi : public WallBase {
 public:
  explicit BingApi()
      : WallBase(),
        m_u8strMft(u8"zh-CN"),
        m_u8strImgNameFmt(u8"{0:%Y-%m-%d} {1}.jpg"),
        m_u8strApiUrl(u8"https://global.bing.com"),
        m_pSetting(nullptr),
        m_uCurImageIndex(0) {
    m_ImageDir = ms_HomePicLocation / u8"必应壁纸";
    InitBase();
  }
  virtual ~BingApi() { delete m_pSetting; }
  virtual bool LoadSetting() override {
    if (fs::exists(m_szSettingPath) &&
        fs::file_size(m_szSettingPath)) {
      m_pSetting = new YJson(m_szSettingPath, YJson::UTF8);
      if (m_pSetting->find(u8"today")->second.getValueString() == GetToday()) {
        m_ImageDir = m_pSetting->find(u8"imgdir")->second.getValueString();
        m_u8strImgNameFmt =
            m_pSetting->find(u8"imgfmt")->second.getValueString();
        m_u8strMft = m_pSetting->find(u8"mkt")->second.getValueString();
        return m_InitOk = true;
      } else {
        delete m_pSetting;
        m_pSetting = nullptr;
        return false;
      }
    }
    return false;
  }
  virtual bool WriteDefaultSetting() override {
    if (!HttpLib::IsOnline())
      return false;
    // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

    std::u8string path(u8"/HPImageArchive.aspx?format=js&idx=0&n=8&mkt=");
    path += m_u8strMft;

    std::u8string body;

    if (HttpLib::Get(m_u8strApiUrl + path, body) != 200)
      return false;

    m_pSetting = new YJson(body.begin(), body.end());
    m_pSetting->append(GetToday(), u8"today");
    m_pSetting->append(m_ImageDir, u8"imgdir");
    m_pSetting->append(m_u8strImgNameFmt, u8"imgfmt");
    m_pSetting->append(m_u8strMft, u8"mkt");
    m_pSetting->append(static_cast<int>(m_uCurImageIndex), u8"index");
    m_pSetting->append(false, u8"auto-download");
    m_pSetting->toFile(m_szSettingPath);
    return m_InitOk = true;
  }
  virtual ImageInfoEx GetNext() override {
    // https://www.bing.com/th?id=OHR.Yellowstone150_ZH-CN055
    // 下这个接口含义，直接看后面的请求参数1084440_UHD.jpg

    ImageInfoEx ptr(new ImageInfo);
    if (!m_InitOk) {
      if (!HttpLib::IsOnline()) {
        ptr->ErrorMsg = u8"Bad network connection.";
        ptr->ErrorCode = ImageInfo::NetErr;
        return ptr;
      } else {
        WriteDefaultSetting();
      }
    }

    auto jsTemp = m_pSetting->find(u8"images")->second.find(m_uCurImageIndex);
    ptr->ImagePath = (m_ImageDir / GetImageName(*jsTemp)).u8string();
    ptr->ImageUrl = m_u8strApiUrl +
                    jsTemp->find(u8"urlbase")->second.getValueString() +
                    u8"_UHD.jpg";
    (*m_pSetting)[u8"index"].second.setValue(static_cast<int>(m_uCurImageIndex));
    m_pSetting->toFile(m_szSettingPath);
    if (++m_uCurImageIndex > 7)
      m_uCurImageIndex = 0;
    ptr->ErrorCode = ImageInfo::NoErr;
    return ptr;
  }
  virtual void SetCurDir(const std::u8string& str) override {
    m_ImageDir = str;
    m_pSetting->find(u8"imgdir")->second.setText(str);
    m_pSetting->toFile(m_szSettingPath);
  }

  virtual void SetJson(bool update) override {
    m_pSetting->toFile(m_szSettingPath);

    // 等待重写
    m_u8strMft = m_pSetting->find(u8"mkt")->second.getValueString();
    m_u8strImgNameFmt = m_pSetting->find(u8"imgfmt")->second.getValueString();
    return;
  }

  virtual YJson* GetJson() override { return m_pSetting; }

 private:
  std::u8string m_u8strMft;
  std::u8string m_u8strImgNameFmt;
  const std::u8string m_u8strApiUrl;
  const char m_szSettingPath[13]{"BingApi.json"};
  YJson* m_pSetting;
  unsigned m_uCurImageIndex;
  std::u8string GetToday() {
    auto utc = chrono::system_clock::now();
    std::string result =
        std::format("{0:%Y-%m-%d}", chrono::current_zone()->to_local(utc));
    return std::u8string(result.begin(), result.end());
  }
  std::u8string GetImageName(YJson& imgInfo) {
    std::string fmt(m_u8strImgNameFmt.begin(), m_u8strImgNameFmt.end());
    std::u8string_view copyright =
        imgInfo.find(u8"copyright")->second.getValueString();
    std::string_view temp(
        reinterpret_cast<const char*>(copyright.data()),
        copyright.find(u8" (© ")
    );
    const auto now =
        chrono::current_zone()->to_local(chrono::system_clock::now());
    std::string result = std::vformat(
        fmt, std::make_format_args(now - chrono::days(m_uCurImageIndex), temp));
    return std::u8string(result.begin(), result.end());
  }
};
