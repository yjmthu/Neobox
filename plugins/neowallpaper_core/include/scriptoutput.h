#include <wallpaper.h>
#include <wallbase.h>

class ScriptOutput : public WallBase {
public:
  explicit ScriptOutput(YJson& setting);
  ~ScriptOutput() override;
  void GetNext(Callback callback) override;
  inline static const auto m_Name = u8"脚本输出"s;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  YJson& GetCurInfo() const
  { return const_cast<ScriptOutput*>(this)->GetCurInfo(); }
};
