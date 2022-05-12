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

#ifdef _WIN32
#define FILE_SEP_PATH "\\"
#elif defined __linux__
#include <sys/stat.h>
#define FILE_SEP_PATH "/"
#endif

#include <QObject>
#include <QDir>
#include <QStandardPaths>

inline void writelog(const std::string& s) {
    std::ofstream f("log.txt", std::ios::binary | std::ios::app);
    f << s << std::endl;
    f.close();
}

#if 0
#define COUT(s) writelog((s))
#else
#define COUT(s)
#endif

typedef std::shared_ptr<std::vector<std::string>> ImageInfo;

class Wallpaper: public QObject
{
    Q_OBJECT

protected:
    void ReadSettings();
    void WriteSettings();
public:
    enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, UNKNOWN };
    explicit Wallpaper();
    virtual ~Wallpaper();
    static bool DownloadImage(const ImageInfo& imageInfo);
    static bool SetWallpaper(const std::string& imagePath);
    static bool PathFileExists(const std::string& filePath);
    static bool PathDirExists(const std::string& dirPath);
    static bool IsImageFile(const std::string& fileName);
    static bool IsOnline();
    static bool IsWorking();
    bool SetNext();
    bool SetPrevious();
    bool SetDropFile(const std::string& filePath);
    inline const std::string& GetCurIamge() const { return m_CurImage; }
    bool RemoveCurrent();
    bool IsPrevAvailable();
    bool IsNextAvailable();
    void SetSlot(int type);
    const std::string& GetImageDir() const;
    int GetTimeInterval() const;
    void SetTimeInterval(int minute);
    int GetImageType();
    bool SetImageType(int index);
    const void* GetDataByName(const char* key) const;
    int GetInt() const;
    std::string GetString() const;
    static constexpr char m_szWallScript[16] { "SetWallpaper.sh" };
private:
    // bool m_IsWorking;
    class QTimer *m_Timer;
    class WallBase* m_Wallpaper;
    std::queue<class WallBase*> m_Jobs;
    std::deque<std::string> m_PrevImgs;
    std::stack<std::string> m_NextImgs;
    std::string m_CurImage;
    bool m_PrevAvailable;
    bool m_NextAvailable;
    bool m_KeepChange;
    static Desktop GetDesktop();
public slots:
    void SetAutoChange(bool flag);
    void SetFirstChange(bool flag);
    void SetCurDir(const std::string& str);
    // void SetValue(const std::string& str, int val);
    // void Update(bool update);
signals:
    void StartTimer(bool start);
};


#endif
