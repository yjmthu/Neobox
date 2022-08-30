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
#include <system_error>

#include "wallpaper.h"

namespace WallClass
{
enum {
    WALLHAVEN = 0, BINGAPI, DIRECTAPI, NATIVE, SCRIPTOUTPUT, FAVORITE};

class Wallhaven;
}

class WallBase
{
  protected:
    std::filesystem::path m_HomePicLocation;
    std::filesystem::path m_ImageDir;
    inline void InitBase()
    {
        if (!LoadSetting())
            WriteDefaultSetting();
    }

  public:
    static WallBase *GetNewInstance(const std::filesystem::path &picHome, int type);
    inline WallBase(const std::filesystem::path &pichome) : m_HomePicLocation(pichome / u8"桌面壁纸")
    {
        if (!std::filesystem::exists(m_HomePicLocation))
        {
            std::filesystem::create_directory(m_HomePicLocation);
        }
    }
    virtual ~WallBase()
    {
    }
    inline const std::filesystem::path &GetImageDir() const
    {
        return m_ImageDir;
    }
    virtual bool LoadSetting() = 0;
    virtual bool WriteDefaultSetting() = 0;
    virtual ImageInfoEx GetNext() = 0;
    virtual void Dislike(const std::u8string &sImgPath);
    virtual void UndoDislike(const std::u8string &sImgPath);
    virtual void SetCurDir(const std::u8string &sImgDir);
    virtual std::u8string GetJson() const = 0;
    virtual void SetJson(const std::u8string &str) = 0;

  private:
    friend class Wallpaper;
    friend class WallClass::Wallhaven;
    static std::atomic_bool m_IsWorking;
};

#endif
