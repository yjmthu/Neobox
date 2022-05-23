#include "speedapp.h"
#include "speedbox.h"
#include "wallpaper/wallpaper.h"

#include <yjson.h>
#include <filesystem>

#include <QDir>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QDebug>

VarBox* m_VarBox = nullptr;

VarBox::VarBox()
{
    m_VarBox = this;
    LoadFonts();
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    constexpr char relPath[] = ".config/Neobox";
    if (dir.exists(relPath) || dir.mkpath(relPath)) dir.cd(relPath);
    QDir::setCurrent(dir.absolutePath());
    GetSetting();
    m_Wallpaper = new Wallpaper;
    int m_BoxExecType = 0;
    m_SpeedBox = new SpeedBox(m_BoxExecType);
    m_SpeedBox->show();
}

VarBox::~VarBox()
{
    delete m_SpeedBox;
    delete m_Setting;
    delete m_Wallpaper;
}

void VarBox::GetSetting()
{
    const char m_szSettingFile[13] { "Setting.json" };
    if (!std::filesystem::exists(m_szSettingFile)) {
        QFile::copy(":/jsons/Setting.json", m_szSettingFile);
        QFile::setPermissions(m_szSettingFile, QFileDevice::ReadUser | QFileDevice::WriteUser);
    }
    m_Setting = new YJson(m_szSettingFile, YJson::UTF8);
    if (!std::filesystem::exists(Wallpaper::m_szWallScript)) {
        QFile::copy(":/scripts/SetWallpaper.sh", Wallpaper::m_szWallScript);
        QFile::setPermissions(Wallpaper::m_szWallScript, QFileDevice::ReadUser | QFileDevice::Permission::ExeUser);
    }
}

void VarBox::SaveSetting()
{
    m_Setting->toFile("Setting.json");
}

void VarBox::LoadFonts()
{
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}
