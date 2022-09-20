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
  explicit Wallpaper(class YJson* settings, void (*callback)(void));
  virtual ~Wallpaper();
  static bool DownloadImage(const ImageInfoEx imageInfo);
  static bool SetWallpaper(const std::filesystem::path& imagePath);
  static bool IsImageFile(const std::filesystem::path& fileName);
  static bool IsWorking();
  bool UndoDelete();
  bool ClearJunk();
  bool SetFavorite();
  bool UnSetFavorite();
  bool SetDropFile(std::deque<std::filesystem::path>&& paths);
  inline const std::filesystem::path GetCurIamge() const { return m_CurImage; }
  void SetSlot(int type);
  const std::filesystem::path& GetImageDir() const;

  bool SetImageType(int type);
  inline int GetImageType() const {
    return m_Settings->find(u8"ImageType")->second.getValueInt();
  }
  void SetFirstChange(bool val);
  inline bool GetFirstChange() const {
    return m_Settings->find(u8"FirstChange")->second.isTrue();
  }
  void SetTimeInterval(int minute);
  inline int GetTimeInterval() const {
    return m_Settings->find(u8"TimeInterval")->second.getValueInt();
  }
  void SetAutoChange(bool val);
  inline bool GetAutoChange() const {
    return m_Settings->find(u8"AutoChange")->second.isTrue();
  }

  static constexpr char m_szWallScript[16]{"SetWallpaper.sh"};

 public:
  static Desktop GetDesktop();
  void SetCurDir(const std::filesystem::path& str);
  void StartTimer(bool start);
  std::filesystem::path m_PicHomeDir;
  class WallBase* m_Wallpaper;

 private:
  YJson* const m_Settings;
  void (*m_SettingsCallback)(void);

  class Timer* m_Timer;
  class WallBase* m_Favorites;
  class WallBase* m_BingWallpaper;
  std::queue<class WallBase*> m_Jobs;
  std::deque<std::filesystem::path> m_PrevImgs;
  std::stack<std::filesystem::path> m_NextImgs;
  std::list<std::pair<std::filesystem::path, std::filesystem::path>>
      m_BlackList;
  std::filesystem::path m_CurImage;
};

#endif
