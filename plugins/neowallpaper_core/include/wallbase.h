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
#include <functional>
#include <system_error>

namespace fs =std::filesystem;

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
  static std::atomic_bool ms_IsWorking;
  static const fs::path ms_HomePicLocation;
  // const std::filesystem::path m_InitImageDir;
  static const fs::path m_DataDir;
  static std::function<void()> SaveSetting;
  static std::string Utf8AsString(const std::u8string& str) { return std::string(str.begin(), str.end()); };
  static std::u8string StringAsUtf8(const std::string& str) { return std::u8string(str.begin(), str.end()); };
  static std::u8string GetStantardDir(const std::u8string& name);

public:
  enum { WALLHAVEN = 0, BINGAPI, DIRECTAPI, NATIVE, SCRIPTOUTPUT, FAVORITE };
  static WallBase* GetNewInstance(YJson& setting, int type);
  static void ClearInstatnce();
  inline explicit WallBase(YJson& setting):
    m_Setting(setting)
    {
      if (!fs::exists(m_DataDir)) {
        fs::create_directory(m_DataDir);
      }
    }
  virtual ~WallBase() {}
  static const fs::path m_ConfigPath;

public:
  virtual fs::path GetImageDir() const = 0;
  virtual ImageInfoEx GetNext() = 0;
  virtual void Dislike(const std::u8string& sImgPath);
  virtual void UndoDislike(const std::u8string& sImgPath);
  virtual void SetCurDir(const std::u8string& sImgDir);
  virtual void SetJson(bool update) = 0;
  YJson& m_Setting;

 private:
  friend class Wallpaper;
};

#endif
