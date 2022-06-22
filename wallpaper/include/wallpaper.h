#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

typedef std::shared_ptr<std::vector<std::u8string>> ImageInfoEx;

class Wallpaper {
 private:
  void ReadSettings();
  void WriteSettings();

 public:
  enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, UNKNOWN };
  explicit Wallpaper(const std::filesystem::path& picHome);
  virtual ~Wallpaper();
  static bool DownloadImage(const ImageInfoEx& imageInfo);
  static bool SetWallpaper(const std::filesystem::path& imagePath);
  static bool IsImageFile(const std::filesystem::path& fileName);
  static bool IsOnline();
  static bool IsWorking();
  bool SetNext();
  bool SetPrevious();
  bool SetDropFile(const std::filesystem::path& filePath);
  inline const std::filesystem::path GetCurIamge() const { return m_CurImage; }
  bool RemoveCurrent();
  bool IsPrevAvailable();
  bool IsNextAvailable();
  void SetSlot(int type);
  const std::filesystem::path& GetImageDir() const;

  bool SetImageType(int type);
  inline int GetImageType() const { return m_ImageType; }
  void SetFirstChange(bool val);
  inline bool GetFirstCHange() const { return m_FirstChange; }
  void SetTimeInterval(int minute);
  inline int GetTimeInterval() const { return m_TimeInterval; }
  void SetAutoChange(bool val);
  inline bool GetAutoChange() const { return m_AutoChange; }

  const void* GetDataByName(const char* key) const;
  static constexpr char m_szWallScript[16]{"SetWallpaper.sh"};

 private:
  int m_ImageType;
  int m_TimeInterval;
  bool m_AutoChange;
  bool m_FirstChange;

  class WallBase* m_Wallpaper;
  std::queue<class WallBase*> m_Jobs;
  std::deque<std::filesystem::path> m_PrevImgs;
  std::stack<std::filesystem::path> m_NextImgs;
  std::filesystem::path m_CurImage;
  bool m_PrevAvailable;
  bool m_NextAvailable;

 public:
  static Desktop GetDesktop();
  void SetCurDir(const std::filesystem::path& str);
  void StartTimer(bool start);
  std::filesystem::path m_PicHomeDir;
  class Timer* m_Timer;
};

#endif
