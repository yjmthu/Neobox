#include <wallpaper.h>
#include <wallbase.h>

class ScriptOutput : public WallBase {
public:
  explicit ScriptOutput(YJson& setting);
  ~ScriptOutput() override;
  void GetNext(std::function<void(ImageInfoEx)> callback) override;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  YJson& GetCurInfo() const
  { return const_cast<ScriptOutput*>(this)->GetCurInfo(); }
};
