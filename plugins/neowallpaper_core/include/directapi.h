#include <wallbase.h>

class DirectApi : public WallBase
{
public:
  explicit DirectApi(YJson& setting);
  ~DirectApi() override;

public:
  ImageInfoEx GetNext() override;
  void SetJson(const YJson& json) override;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  const YJson& GetCurInfo() const {
    return const_cast<DirectApi*>(this)->GetCurInfo();
  }
  std::u8string GetImageName();
};
