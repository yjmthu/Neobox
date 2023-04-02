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
#include <wallconfig.h>

namespace fs = std::filesystem;

enum class OperatorType {
  Next, UNext, Dislike, UDislike, Favorite, UFavorite
};


class Wallpaper {
  enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, UNKNOWN };
public:
  using Locker = WallBase::Locker;
  using LockerEx = WallBase::LockerEx;
  static bool DownloadImage(const ImageInfoEx imageInfo);
  static bool IsImageFile(const fs::path& fileName);
private:
  static bool SetWallpaper(fs::path imagePath);
  static Desktop GetDesktop();
  fs::path Url2Name(const std::u8string& url);
  void ReadSettings();
  void WriteSettings();
  void AppendBlackList(const fs::path& path);
  void WriteBlackList();
  bool PushBack(ImageInfoEx ptr);
  bool MoveRight();
private:
  void SetNext();
  void UnSetNext();
  void SetDislike();
  void UnSetDislike();
  void SetFavorite();
  void UnSetFavorite();

public:
  void SetDropFile(std::u8string url);
  void UpdateRegString(bool forward=false);
  void SetSlot(OperatorType type);
  bool SetImageType(int type);
  void SetFirstChange(bool val);
  void SetTimeInterval(int minute);
  void SetAutoChange(bool val);
  void ClearJunk();
  const fs::path& GetCurIamge() const { return m_CurImage; }
  WallBase* Engine() { return m_Wallpaper; }

public:
  WallConfig m_Settings;

private:
  YJson* const m_Config;
#ifdef _WIN32
  typedef std::wstring String;
#else
  typedef std::string String;
#endif
  static const String m_ImgNamePattern;
  static constexpr char m_szWallScript[16]{"SetWallpaper.sh"};

public:
  explicit Wallpaper(class YJson& settings);
  virtual ~Wallpaper();

private:
  std::mutex m_DataMutex;
  std::mutex m_ThreadMutex;

  class NeoTimer* const m_Timer;
  class WallBase* const m_Favorites;
  class WallBase* const m_BingWallpaper;
  class WallBase* m_Wallpaper;
  std::deque<fs::path> m_PrevImgs;
  std::stack<fs::path> m_NextImgs;
  std::list<std::pair<fs::path, fs::path>> m_BlackList;
  fs::path m_CurImage;
};

#endif
