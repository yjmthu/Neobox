#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <filesystem>

class BingApi : public WallBase {
public:
  explicit BingApi(YJson& setting);
  virtual ~BingApi();

public:
  void GetNext(std::function<void(ImageInfoEx)> callback) override;
  void SetJson(const YJson& json) override;

private:
  void AutoDownload();
  YJson& InitSetting(YJson& setting);
  void InitData();
  void CheckData(std::function<void()> cbOK, std::function<void()> cbNO);
  static std::u8string GetToday();
  std::u8string GetImageName(YJson& imgInfo);

private:
  YJson* m_Data;
  std::unique_ptr<class HttpLib> m_DataRequest;
  const fs::path m_DataPath = m_DataDir / u8"BingApiData.json";
};
