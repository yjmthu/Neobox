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
  ImageInfoEx GetNext() override;
  void SetJson(YJson json) override;

private:
  void AutoDownload();
  YJson& InitSetting(YJson& setting);
  void InitData();
  bool CheckData();
  static std::u8string GetToday();
  std::u8string GetImageName(YJson& imgInfo);

private:
  YJson* m_Data;
  const fs::path m_DataPath = m_DataDir / u8"BingApiData.json";
};
