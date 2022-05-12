#ifndef APICLASS_H
#define APICLASS_H

#define BOOST_SPIRIT_THREADSAFE
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#elif defined(__linux__)
// #include <sys/io.h>
#include <dirent.h>
#endif
#include <time.h>
#include <sstream>
#include <string>
#include <fstream>
#include <random>
#include <system_error>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "wallpaper.h"
#include "3rdlib/httplib/httplib.hpp"

#include <yjson.h>

inline std::string ToUtf8(const std::string& path)
{
    return path;
}

namespace WallClass {
    class Wallhaven;
}

class WallBase {
protected:
    std::string m_HomePicLocation;
    std::string m_ImageDir;
    inline void InitBase() { if (!LoadSetting()) WriteDefaultSetting(); }
public:
    static WallBase* GetNewInstance(int type);
    inline WallBase(): m_HomePicLocation(
        QDir::toNativeSeparators( 
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            ).toStdString() + u8"" FILE_SEP_PATH "桌面壁纸") {
        if (!Wallpaper::PathDirExists(m_HomePicLocation)) {
            mkdir(m_HomePicLocation.c_str(), 0777);
        }
    }
    virtual ~WallBase(){ }
    inline const std::string& GetImageDir() {return m_ImageDir;}
    virtual bool LoadSetting() = 0;
    virtual bool WriteDefaultSetting() = 0;
    virtual ImageInfo GetNext() = 0;
    virtual void Dislike(const std::string& img) = 0;
    virtual void SetCurDir(const std::string& str) {}
    virtual const void* GetDataByName(const char* key) const {
        return nullptr;
    }
    // virtual void SetValue(const std::string& key, int val) {}
    virtual std::string GetString() const { return std::string(); }
    virtual int GetInt() const { return 0; }
    virtual void Update(bool update) {}
    // std::shared_ptr<int> sp3(new int[10](), std::default_delete<int[]>());
private:
    friend class Wallpaper;
    friend class WallClass::Wallhaven;
    static bool m_IsWorking;
    // static int m_JobLeft;
};

#endif
