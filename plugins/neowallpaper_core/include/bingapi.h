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
  void SetCurDir(const std::u8string& str) override;
  void SetJson(bool update) override;
  fs::path GetImageDir() const override;

private:
  void AutoDownload();
  YJson& InitSetting(YJson& setting);
  void InitData();
  bool CheckData();
  static std::u8string GetToday();
  std::u8string GetImageName(YJson& imgInfo);

private:
  YJson* m_Data;
  // const std::u8string m_u8strApiUrl;
  const fs::path m_DataPath = m_DataDir / u8"BingApiData.json";
};
