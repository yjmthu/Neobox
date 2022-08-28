#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>

#include <QBitmap>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTimer>
#include <appcode.hpp>
#include <filesystem>

VarBox *m_VarBox = nullptr;

inline void MessageBox(const std::u8string &msg)
{
    QString str(QString::fromUtf8(reinterpret_cast<const char *>(msg.data()), static_cast<int>(msg.size())));
    QMessageBox::information(nullptr, "Notice", str);
}

VarBox::VarBox() : QObject()
{
    m_VarBox = this;
    LoadFonts();
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    constexpr char relPath[] = ".config/Neobox";
    if (dir.exists(relPath) || dir.mkpath(relPath))
        dir.cd(relPath);
    QDir::setCurrent(dir.absolutePath());
    LoadSettings();
    LoadQmlFiles();
}

VarBox::~VarBox()
{
}

void VarBox::LoadSettings()
{
#ifdef _WIN32
#elif def __linux__
    if (!std::filesystem::exists(Wallpaper::m_szWallScript))
    {
        QFile::copy(":/scripts/SetWallpaper.sh", Wallpaper::m_szWallScript);
        QFile::setPermissions(Wallpaper::m_szWallScript, QFileDevice::ReadUser | QFileDevice::Permission::ExeUser);
    }
#endif
}

void VarBox::LoadFonts()
{
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}

void VarBox::LoadQmlFiles()
{
    QString prefix(":/");
    if (!std::filesystem::exists("qmls"))
    {
        std::filesystem::create_directory("qmls");
    }
    auto lst = {"qmls/FloatingWindow.qml", "qmls/MainMenu.qml", "qmls/NeoMenuItem.qml", "qmls/SystemTray.qml"};
    for (const auto &i : lst)
    {
        if (!QFile::exists(i))
        {
            QFile::copy(prefix + i, i);
            QFile::setPermissions(i, QFileDevice::ReadUser);
        }
    }
}
