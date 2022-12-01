#include <wallbase.h>

class Native : public WallBase {
private:
  size_t GetFileCount();
  bool GetFileList();

public:
  explicit Native(YJson& setting);
  virtual ~Native();

public:
  ImageInfoEx GetNext() override;
  void SetCurDir(const std::u8string& str) override;
  void SetJson(bool update) override;
  fs::path GetImageDir() const override;

private:
  YJson& InitSetting(YJson& setting);
  YJson& GetCurInfo();
  YJson& GetCurInfo() const {
    return const_cast<Native*>(this)->GetCurInfo();
  }

private:
  std::vector<std::u8string> m_FileList;
};
