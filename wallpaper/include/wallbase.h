#ifndef APICLASS_H
#define APICLASS_H

#include <stdio.h>
#include <time.h>
#include <yjson.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <system_error>

struct ImageInfo {
  enum Errors : uint32_t { NoErr, NetErr, FileErr, RunErr, CfgErr, DataErr };
  std::u8string ImagePath;
  std::u8string ImageUrl;
  std::u8string ErrorMsg;
  uint32_t ErrorCode;
};

typedef std::shared_ptr<ImageInfo> ImageInfoEx;

class WallBase {
 protected:
  bool m_InitOk = false;
  static const std::filesystem::path ms_HomePicLocation;
  std::filesystem::path m_ImageDir;
  inline void InitBase() {
    if (!LoadSetting())
      WriteDefaultSetting();
  }

 public:
  enum { WALLHAVEN = 0, BINGAPI, DIRECTAPI, NATIVE, SCRIPTOUTPUT, FAVORITE };
  static WallBase* GetNewInstance(int type);
  static void ClearInstatnce();
  inline WallBase() {
    if (!std::filesystem::exists(ms_HomePicLocation)) {
      std::filesystem::create_directory(ms_HomePicLocation);
    }
  }
  virtual ~WallBase() {}
  inline const std::filesystem::path& GetImageDir() const { return m_ImageDir; }
  virtual bool LoadSetting() = 0;
  virtual bool WriteDefaultSetting() = 0;
  virtual ImageInfoEx GetNext() = 0;
  virtual void Dislike(const std::u8string& sImgPath);
  virtual void UndoDislike(const std::u8string& sImgPath);
  virtual void SetCurDir(const std::u8string& sImgDir);
  virtual YJson* GetJson() = 0;
  virtual void SetJson(bool update) = 0;

 private:
  friend class Wallpaper;
  static std::atomic_bool m_IsWorking;
};

#endif
