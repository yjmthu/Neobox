#include "speedapp.h"
#include "speedbox.h"
#include "wallpaper/wallpaper.h"

#include <yjson.h>

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
    if (Wallpaper::PathFileExists(m_szSettingFile)) {
        m_Setting = new YJson(m_szSettingFile, YJson::UTF8);
    } else {
        m_Setting = new YJson("{\"Wallpaper\":"
                                  "{\"AutoChange\":false,\"FirstChange\":false,\"TimeInterval\":15,\"ImageType\":0},"
                              "\"FormUi\":{\"BkColor\":[0,0,0,200]}}");
        m_Setting->toFile(m_szSettingFile, YJson::UTF8, true);
    }
}

void VarBox::LoadFonts()
{
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}
