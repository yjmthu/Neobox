#include <wallpaper.h>
#include <wallbase.h>

class ScriptOutput : public WallBase {
public:
  explicit ScriptOutput(YJson& setting);
  ~ScriptOutput() override;
  ImageInfoEx GetNext() override;
  void SetCurDir(const std::u8string &sImgDir) override;
  void SetJson(bool update) override;
  fs::path GetImageDir() const override;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  YJson& GetCurInfo() const {
    return const_cast<ScriptOutput*>(this)->GetCurInfo();
  }
};
