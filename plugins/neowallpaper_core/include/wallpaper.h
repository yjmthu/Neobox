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

class Wallpaper {
 private:
  void ReadSettings();
  void WriteSettings() const;
  void AppendBlackList(const std::filesystem::path& path);
  void WriteBlackList() const;
  bool SetNext();
  bool SetPrevious();
  bool RemoveCurrent();

 public:
  enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, UNKNOWN };
  explicit Wallpaper(class YJson& settings, std::function<void()>);
  virtual ~Wallpaper();
  static bool DownloadImage(const ImageInfoEx imageInfo);
  static bool SetWallpaper(std::filesystem::path imagePath);
  static bool IsImageFile(const std::filesystem::path& fileName);
  static bool IsWorking();
  bool UndoDelete();
  bool ClearJunk();
  bool SetFavorite();
  bool UnSetFavorite();
  bool SetDropFile(std::vector<std::u8string> urls);
  inline const std::filesystem::path& GetCurIamge() const { return m_CurImage; }
  void SetSlot(int type);
  const std::filesystem::path& GetImageDir() const;

  bool SetImageType(int type);
  inline int GetImageType() const {
    return m_Settings[u8"ImageType"].getValueInt();
  }
  void SetFirstChange(bool val);
  inline bool GetFirstChange() const {
    return m_Settings[u8"FirstChange"].isTrue();
  }
  void SetTimeInterval(int minute);
  inline int GetTimeInterval() const {
    return m_Settings[u8"TimeInterval"].getValueInt();
  }
  void SetAutoChange(bool val);
  inline bool GetAutoChange() const {
    return m_Settings[u8"AutoChange"].isTrue();
  }

  static constexpr char m_szWallScript[16]{"SetWallpaper.sh"};

 public:
  static Desktop GetDesktop();
  void SetCurDir(std::filesystem::path str);
  void StartTimer(bool start);
  // std::filesystem::path m_PicHomeDir;
  static const std::wstring m_ImgNamePattern;
  std::list<std::u8string> m_NextImgsBuffer;
  class WallBase* m_Wallpaper;

 private:
  YJson& m_Settings;
  std::function<void()> SettingsCallback;  // call this to save settings
  std::filesystem::path GetImageName(const std::u8string& url);

  class NeoTimer* const m_Timer;
  class WallBase* const m_Favorites;
  class WallBase* const m_BingWallpaper;
  std::deque<std::filesystem::path> m_PrevImgs;
  std::stack<std::filesystem::path> m_NextImgs;
  std::list<std::pair<std::filesystem::path, std::filesystem::path>>
      m_BlackList;
  std::filesystem::path m_CurImage;
};

#endif
