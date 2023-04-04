#include <wallbase.h>

class WallhavenData {
public:
  explicit WallhavenData(fs::path path)
    : m_DataPath(std::move(path))
    , m_Data(InitData())
    , m_ApiUrl(m_Data[u8"Api"].getValueString())
    , m_Used(m_Data[u8"Used"].getArray())
    , m_Unused(m_Data[u8"Unused"].getArray())
    , m_Blacklist(m_Data[u8"Blacklist"].getArray())
  {}
private:
  const fs::path m_DataPath;
  YJson m_Data;
  YJson InitData();
public:
  std::u8string& m_ApiUrl;
  YJson::ArrayType& m_Used;
  YJson::ArrayType& m_Unused;
  YJson::ArrayType& m_Blacklist;
  bool IsEmpty() const;
  void ClearAll();
  void SaveData();
};

class Wallhaven : public WallBase {
 private:
  static bool IsPngFile(std::u8string& str);
 public:
  ImageInfoEx GetNext() override;
  void Dislike(std::u8string_view sImgPath) override;
  void UndoDislike(std::u8string_view sImgPath) override;
public:
  explicit Wallhaven(YJson& setting);
  virtual ~Wallhaven();
  void SetJson(const YJson& json) override;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  YJson& GetCurInfo() const
  { return const_cast<Wallhaven*>(this)->GetCurInfo(); };

  std::string IsWallhavenFile(std::string name);
  size_t DownloadUrl(std::u8string mainUrl);
  bool CheckData(ImageInfoEx ptr);
  std::u8string GetApiPathUrl() const;
private:
  // const char m_szDataPath[13]{"ImgData.json"};
  WallhavenData m_Data;
};
