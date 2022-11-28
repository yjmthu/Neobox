#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
#include <sstream>
#include <filesystem>
#include <thread>

// export module wallpaper1;

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using namespace std::literals;

/* export */ class BingApi : public WallBase {
 public:
  explicit BingApi()
      : WallBase(true),
        m_u8strMft(u8"zh-CN"),
        m_u8strImgNameFmt(u8"{0:%Y-%m-%d} {1}.jpg"),
        m_u8strApiUrl(u8"https://global.bing.com"),
        m_pSetting(nullptr) {
    m_ImageDir = ms_HomePicLocation / u8"必应壁纸";
    InitBase();
    AutoDownload();
  }
  virtual ~BingApi() { delete m_pSetting; }
  virtual bool LoadSetting() override {
    if (fs::exists(m_szSettingPath) &&
        fs::file_size(m_szSettingPath)) {
      m_pSetting = new YJson(m_szSettingPath, YJson::UTF8);
      auto& today = m_pSetting->find(u8"today")->second.getValueString();
      m_ImageDir = m_pSetting->find(u8"imgdir")->second.getValueString();
      m_u8strImgNameFmt = m_pSetting->find(u8"imgfmt")->second.getValueString();
      m_u8strMft = m_pSetting->find(u8"mkt")->second.getValueString();
      if (today == GetToday() && !m_pSetting->find(u8"images")->second.emptyA()) {
        return m_InitOk = true;
      } else {
        today = GetToday();
        m_pSetting->find(u8"images")->second.clearA();
        m_pSetting->toFile(m_szSettingPath);
        return false;
      }
    }
    m_pSetting = new YJson {
      YJson::O {
        { u8"images"sv, YJson::Array },
        { u8"today"sv, GetToday() },
        { u8"imgdir"sv, m_ImageDir.u8string() },
        { u8"imgfmt"sv, m_u8strImgNameFmt },
        { u8"mkt"sv, m_u8strMft },
        { u8"auto-download"sv, false }
      }
    };
    m_pSetting->toFile(m_szSettingPath);
    return false;
  }
  virtual bool WriteDefaultSetting() override {
    if (!HttpLib::IsOnline())
      return false;
    // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

    HttpLib clt(m_u8strApiUrl + u8"/HPImageArchive.aspx?format=js&idx=0&n=8&mkt="s + m_u8strMft);
    auto res = clt.Get();

    if (res->status != 200)
      return false;

    YJson data(res->body.begin(), res->body.end());
    YJson::swap(m_pSetting->find(u8"images")->second, data[u8"images"]);
    m_pSetting->toFile(m_szSettingPath);

    return m_InitOk = true;
  }
  virtual ImageInfoEx GetNext() override {
    static size_t s_uCurImgIndex = 0;

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
    } else if (m_pSetting->find(u8"today")->second.getValueString() != GetToday()) {
      if (WriteDefaultSetting()) {
        m_pSetting->find(u8"today")->second = GetToday();
        s_uCurImgIndex = 0;
        m_pSetting->toFile(m_szSettingPath);
      } else {
        ptr->ErrorMsg = u8"Bad network connection.";
        ptr->ErrorCode = ImageInfo::NetErr;
        return ptr;
      }
    }

    try {
      auto& jsTemp = m_pSetting->find(u8"images")->second[s_uCurImgIndex];
      ptr->ImagePath = (m_ImageDir / GetImageName(jsTemp)).u8string();
      ptr->ImageUrl = m_u8strApiUrl + jsTemp[u8"urlbase"].getValueString() + u8"_UHD.jpg";
      m_pSetting->toFile(m_szSettingPath);
      ++s_uCurImgIndex &= 0x07;
      ptr->ErrorCode = ImageInfo::NoErr;
    } catch (...) {
      ptr->ErrorCode = ImageInfo::DataErr;
    }
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
  void AutoDownload() {
    if (m_pSetting->find(u8"auto-download"sv)->second.isFalse())
      return;
    std::thread([this](){
      std::this_thread::sleep_for(30s);
      while (!HttpLib::IsOnline() || ms_IsWorking) {
        std::this_thread::sleep_for(1s);
      }

      ms_IsWorking = true;
      if (!m_InitOk) {
        WriteDefaultSetting();
      }
      for (auto& item: m_pSetting->find(u8"images")->second.getArray()) {
        ImageInfoEx ptr(new ImageInfo);
        ptr->ImagePath = (m_ImageDir / GetImageName(item)).u8string();
        ptr->ImageUrl = m_u8strApiUrl +
                        item[u8"urlbase"].getValueString() +
                        u8"_UHD.jpg";
        ptr->ErrorCode = ImageInfo::NoErr;
        Wallpaper::DownloadImage(ptr);
      }
      ms_IsWorking = false;
    }).detach();
  }

  std::u8string m_u8strMft;
  std::u8string m_u8strImgNameFmt;
  const std::u8string m_u8strApiUrl;
  const char m_szSettingPath[13]{"BingApi.json"};
  YJson* m_pSetting;
  std::u8string GetToday() {
    auto utc = chrono::system_clock::now();
    std::string result =
        std::format("{0:%Y-%m-%d}", chrono::current_zone()->to_local(utc));
    return std::u8string(result.begin(), result.end());
  }
  std::u8string GetImageName(YJson& imgInfo) {
    // see https://codereview.stackexchange.com/questions/156695/converting-stdchronotime-point-to-from-stdstring
    const std::string fmt(m_u8strImgNameFmt.begin(), m_u8strImgNameFmt.end());
    std::u8string_view date = imgInfo[u8"enddate"].getValueString();
    std::u8string_view copyright =
        imgInfo.find(u8"copyright")->second.getValueString();
    std::string_view temp(
        reinterpret_cast<const char*>(copyright.data()),
        copyright.find(u8" (© ")
    );

    std::tm tm {};
    std::istringstream(std::string(date.begin(), date.end())) >> std::get_time(&tm, "%Y%m%d");
    chrono::system_clock::time_point timePoint  = {};
    timePoint += chrono::seconds(std::mktime(&tm)) + 24h;

    std::string result = std::vformat(fmt, std::make_format_args(timePoint, temp));
    return std::u8string(result.begin(), result.end());
  }
};
