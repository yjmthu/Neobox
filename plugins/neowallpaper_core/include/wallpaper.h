#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <functional>

#include <wallbase.h>
#include <yjson.h>

namespace fs = std::filesystem;

class Wallpaper {
 private:
  void ReadSettings();
  void WriteSettings() const;
  void AppendBlackList(const fs::path& path);
  void WriteBlackList() const;
  bool SetNext();
  bool SetPrevious();
  bool RemoveCurrent();

 private:
  YJson& m_Settings;
  YJson* const m_Config;

 public:
  enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, UNKNOWN };
  explicit Wallpaper(class YJson& settings, std::function<void()>);
  virtual ~Wallpaper();
  static bool DownloadImage(const ImageInfoEx imageInfo);
  static bool SetWallpaper(fs::path imagePath);
  static bool IsImageFile(const fs::path& fileName);
  static bool IsWorking();
  bool UndoDelete();
  bool ClearJunk();
  bool SetFavorite();
  bool UnSetFavorite();
  bool SetDropFile(std::vector<std::wstring> urls);
  const fs::path& GetCurIamge() const { return m_CurImage; }
  void SetSlot(int type);

  bool SetImageType(int type);
  int GetImageType() const {
    return m_Settings[u8"ImageType"].getValueInt();
  }
  void SetFirstChange(bool val);
  bool GetFirstChange() const {
    return m_Settings[u8"FirstChange"].isTrue();
  }
  void SetTimeInterval(int minute);
  int GetTimeInterval() const {
    return m_Settings[u8"TimeInterval"].getValueInt();
  }
  void SetAutoChange(bool val);
  bool GetAutoChange() const {
    return m_Settings[u8"AutoChange"].isTrue();
  }

  static constexpr char m_szWallScript[16]{"SetWallpaper.sh"};

 public:
  static Desktop GetDesktop();
  void StartTimer(bool start);
  // fs::path m_PicHomeDir;
  static const std::wstring m_ImgNamePattern;
  std::list<std::wstring> m_NextImgsBuffer;
  class WallBase* m_Wallpaper;

 private:
  std::function<void()> SettingsCallback;  // call this to save settings
  fs::path GetImageName(const std::wstring& url);

  class NeoTimer* const m_Timer;
  class WallBase* const m_Favorites;
  class WallBase* const m_BingWallpaper;
  std::deque<fs::path> m_PrevImgs;
  std::stack<fs::path> m_NextImgs;
  std::list<std::pair<fs::path, fs::path>> m_BlackList;
  fs::path m_CurImage;
};

#endif
