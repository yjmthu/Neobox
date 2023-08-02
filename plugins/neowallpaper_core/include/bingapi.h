#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <filesystem>

class BingApi : public WallBase {
  typedef std::function<void()> CheckCallback;
public:
  explicit BingApi(YJson& setting);
  virtual ~BingApi();

public:
  void GetNext(Callback callback) override;
  void SetJson(const YJson& json) override;

private:
  void AutoDownload();
  YJson& InitSetting(YJson& setting);
  void InitData();
  void CheckData(CheckCallback cbOK, std::optional<CheckCallback> cbNO);
  static std::u8string GetToday();
  std::u8string GetImageName(YJson& imgInfo);
  class NeoTimer* m_Timer;

private:
  YJson* m_Data;
  std::unique_ptr<class HttpLib> m_DataRequest;
  const fs::path m_DataPath = m_DataDir / u8"BingApiData.json";
};
