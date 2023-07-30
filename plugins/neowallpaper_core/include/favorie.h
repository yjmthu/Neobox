#include <wallpaper.h>
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
  // void InitData();
  void GetNext(std::function<void(ImageInfoEx)> callback) override;
  fs::path GetImageDir() const;

  void Dislike(std::u8string_view sImgPath) override;
  void UndoDislike(std::u8string_view sImgPath) override;

private:
  size_t GetFileCount() const;
  bool GetFileList();

private:
  std::vector<std::u8string> m_FileList;
};
