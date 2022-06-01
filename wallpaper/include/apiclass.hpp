#ifndef APICLASS_H
#define APICLASS_H

#define BOOST_SPIRIT_THREADSAFE
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <time.h>
#include <sstream>
#include <string>
#include <fstream>
#include <random>
#include <filesystem>
#include <system_error>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "wallpaper.h"
#include <httplib.hpp>

#include <yjson.h>

namespace WallClass {
    class Wallhaven;
}

class WallBase {
protected:
    std::filesystem::path m_HomePicLocation;
    std::filesystem::path m_ImageDir;
    inline void InitBase() { if (!LoadSetting()) WriteDefaultSetting(); }
public:
    static WallBase* GetNewInstance(const std::filesystem::path& picHome, int type);
    inline WallBase(const std::filesystem::path& pichome)
        : m_HomePicLocation(pichome / u8"桌面壁纸")
    {
        if (!std::filesystem::exists(m_HomePicLocation)) {
            std::filesystem::create_directory(m_HomePicLocation);
        }
    }
    virtual ~WallBase(){ }
    inline const std::filesystem::path& GetImageDir() const
        { return m_ImageDir; }
    virtual bool LoadSetting() = 0;
    virtual bool WriteDefaultSetting() = 0;
    virtual ImageInfoEx GetNext() = 0;
    virtual void Dislike(const std::filesystem::path& img) = 0;
    virtual void SetCurDir(const std::filesystem::path& str) = 0;
    virtual const void* GetDataByName(const char* key) const {
        return nullptr;
    }
    virtual std::u8string GetString() const { return std::u8string(); }
    virtual int GetInt() const { return 0; }
    virtual void Update(bool update) {}
private:
    friend class Wallpaper;
    friend class WallClass::Wallhaven;
    static bool m_IsWorking;
};

#endif
