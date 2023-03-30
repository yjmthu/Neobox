#include <wallbase.h>

class Wallhaven : public WallBase {
 private:
  static bool IsPngFile(std::u8string& str);
 public:
  ImageInfoEx GetNext() override;
  void Dislike(const std::u8string& sImgPath) override;
  void UndoDislike(const std::u8string& sImgPath) override;
public:
  explicit Wallhaven(YJson& setting);
  virtual ~Wallhaven();
  void SetJson(YJson json) override;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  YJson& GetCurInfo() const {
    return const_cast<Wallhaven*>(this)->GetCurInfo();
  };
  std::string IsWallhavenFile(std::string name);
  size_t DownloadUrl(const std::u8string& mainUrl);
  bool CheckData(ImageInfoEx ptr);
  std::u8string GetApiPathUrl() const;
private:
  // const char m_szDataPath[13]{"ImgData.json"};
  YJson* m_Data;
  const fs::path m_DataPath = m_DataDir / u8"WallhaveData.json";
};
