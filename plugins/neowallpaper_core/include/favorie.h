#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
#include <functional>
#include <filesystem>

class Favorite : public WallBase {
public:
  explicit Favorite(YJson& setting);
  ~Favorite();
public:
  YJson& InitSetting(YJson& setting);
  void InitData();
  ImageInfoEx GetNext() override;
  fs::path GetImageDir() const;

  void Dislike(const std::u8string& sImgPath) override;
  void UndoDislike(const std::u8string& sImgPath) override;

  void SetJson(bool update) override;

private:
  YJson* m_Data;
  const fs::path m_DataPath = m_DataDir / u8"FavoriteData.json";
};
