#include "speedapp.h"
#include "speedbox.h"
#include "wallpaper/wallpaper.h"
#include "speedmenu.h"
#include "translate/translater.h"

#include <yjson.h>
#include <filesystem>

#include <QDir>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QSystemTrayIcon>

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
    m_Tray = new QSystemTrayIcon;
    m_Wallpaper = new Wallpaper;
    m_SpeedBox = new SpeedBox;
    m_Menu = new SpeedMenu;
    m_Menu->showEvent(nullptr);
    m_Tray->setContextMenu(m_Menu);
    m_Tray->setIcon(QIcon(QStringLiteral(":/icons/speedbox.ico")));
    m_SpeedBox->SetupUi();
    m_Tray->show();
    if (m_VarBox->m_Setting->find(u8"FormGlobal")->second[u8"ShowForm"].second.isTrue())
        m_SpeedBox->show();
    m_Translater = new Translater;
    QObject::connect(m_Tray, &QSystemTrayIcon::activated, m_Translater, &Translater::IntelligentShow);
}

VarBox::~VarBox()
{
    delete m_Translater;
    delete m_Menu;
    delete m_SpeedBox;
    delete m_Wallpaper;
    delete m_Tray;
    delete m_Setting;
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
