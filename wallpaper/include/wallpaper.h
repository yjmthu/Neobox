#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <queue>
#include <deque>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>

typedef std::shared_ptr<std::vector<std::u8string>> ImageInfoEx;

class Wallpaper
{
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
    inline const std::filesystem::path GetCurIamge() const
        { return m_CurImage; }
    bool RemoveCurrent();
    bool IsPrevAvailable();
    bool IsNextAvailable();
    void SetSlot(int type);
    const std::filesystem::path& GetImageDir() const;
    int GetTimeInterval() const;
    void SetTimeInterval(int minute);
    int GetImageType();
    bool SetImageType(int index);
    const void* GetDataByName(const char* key) const;
    int GetInt() const;
    std::u8string GetString() const;
    static constexpr char m_szWallScript[16] { "SetWallpaper.sh" };
private:
    class YJson*const m_Setting;
    class WallBase *m_Wallpaper;
    std::queue<class WallBase*> m_Jobs;
    std::deque<std::filesystem::path> m_PrevImgs;
    std::stack<std::filesystem::path> m_NextImgs;
    std::filesystem::path m_CurImage;
    bool m_PrevAvailable;
    bool m_NextAvailable;
    bool m_KeepChange;
public:
    static Desktop GetDesktop();
    void SetAutoChange(bool flag);
    void SetFirstChange(bool flag);
    void SetCurDir(const std::filesystem::path& str);
    void StartTimer(bool start);
    std::filesystem::path m_PicHomeDir;
    class Timer* m_Timer;
};


#endif
