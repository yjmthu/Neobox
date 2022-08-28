#include <pixmapimage.h>
#include <qxtglobalshortcut.h>
#include <speedbox.h>
#include <speedmenu.h>
#include <sysapi.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QStandardPaths>
#include <QtQuick>
#include <appcode.hpp>
#include <regex>
#include <thread>

using namespace std::literals;

SpeedMenu::SpeedMenu() : QObject(nullptr)
{
    auto picHome =
        QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toUtf8();
    m_Wallpaper = new Wallpaper(std::u8string(picHome.begin(), picHome.end()));
}

SpeedMenu::~SpeedMenu()
{
    delete m_Wallpaper;
}

bool SpeedMenu::appAutoStart()
{
#ifdef WIN32
    return !QSettings(QStringLiteral("HKEY_CURRENT_"
                                     "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
                      QSettings::NativeFormat)
                .value("Neobox")
                .toString()
                .compare(QDir::toNativeSeparators(qApp->applicationFilePath()));
#else
    QString m_AutoStartFile =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/autostart/Neobox.desktop";
    if (!QFile::exists(m_AutoStartFile))
    {
        return false;
    }
    else
    {
        QSettings m_Setting(m_AutoStartFile, QSettings::IniFormat);
        m_Setting.beginGroup("Desktop Entry");
        QString str = m_Setting.value("Exec", "null").toString();
        m_Setting.endGroup();
        return str.indexOf(qApp->applicationFilePath()) != -1;
    }
#endif
}

void SpeedMenu::appSetAutoStart(bool start)
{
#ifdef WIN32
    QSettings reg(QStringLiteral("HKEY_CURRENT_"
                                 "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
                  QSettings::NativeFormat);
    if (start)
        reg.setValue("Neobox", QDir::toNativeSeparators(qApp->applicationFilePath()));
    else
        reg.remove("Neobox");
#else
    QString configLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    std::filesystem::path desktopFolderName = (configLocation + "/autostart").toStdU16String();
    if (!std::filesystem::exists(desktopFolderName))
        std::filesystem::create_directory(desktopFolderName);
    QString iconFileName = configLocation + "/Neobox/Neobox.ico";
    if (start)
    {
        if (!QFile::exists(iconFileName))
        {
            QFile::copy(":/icons/speedbox.ico", iconFileName);
            QFile::setPermissions(iconFileName, QFileDevice::ReadUser);
        }
        std::ofstream file(desktopFolderName / "Neobox.desktop"s, std::ios::binary | std::ios::out);
        file << "[Desktop Entry]"sv << std::endl
             << "Name=Neobox" << std::endl
             << "Comment=A powerful wallpaper tool." << std::endl
             << "Icon=" << iconFileName.toStdString() << std::endl
             << "Exec=sh -c \"(sleep 3 && exec \'" << qApp->applicationFilePath().toStdString() << "\' -b)\""
             << std::endl
             << "Categories=System" << std::endl
             << "Terminal=false" << std::endl
             << "Type=Application" << std::endl;
        file.close();
    }
    else if (std::filesystem::exists(desktopFolderName /= "Neobox.desktop"s))
    {
        std::filesystem::remove(desktopFolderName);
    }
#endif // WIN32
    emit appAutoStartChanged();
}

int SpeedMenu::wallpaperType() const
{
    return m_Wallpaper->GetImageType();
}

void SpeedMenu::wallpaperSetType(int type)
{
    if (type == m_Wallpaper->GetImageType())
        return;
    emit wallpaperTypeChanged(m_Wallpaper->SetImageType(type));
}

int SpeedMenu::wallpaperTimeInterval() const
{
    return m_Wallpaper->GetTimeInterval();
}

void SpeedMenu::wallpaperSetTimeInterval(int minute)
{
    m_Wallpaper->SetTimeInterval(minute);
    emit wallpaperTimeIntervalChanged();
}

QString SpeedMenu::wallpaperDir() const
{
    return QString::fromStdU16String(m_Wallpaper->GetImageDir().u16string());
}

void SpeedMenu::wallpaperSetDir(const QString &str)
{
    QByteArray &&array = str.toUtf8();
#ifdef _WIN32
    m_Wallpaper->SetCurDir(std::u8string(array.begin() + 8, array.end()));
#else
    m_Wallpaper->SetCurDir(std::u8string(array.begin() + 7, array.end()));
#endif
    emit wallpaperDirChanged();
}

bool SpeedMenu::wallpaperAutoChange() const
{
    return m_Wallpaper->GetAutoChange();
}

void SpeedMenu::wallpaperSetAutoChange(bool val)
{
    m_Wallpaper->SetAutoChange(val);
    emit wallpaperAutoChangeChanged();
}

bool SpeedMenu::wallpaperFirstChange() const
{
    return m_Wallpaper->GetFirstCHange();
}

void SpeedMenu::wallpaperSetFirstChange(bool val)
{
    m_Wallpaper->SetFirstChange(val);
    emit wallpaperFirstChangeChanged();
}

void SpeedMenu::wallpaperUndoDelete()
{
    m_Wallpaper->UndoDelete();
}

void SpeedMenu::wallpaperClearJunk()
{
    m_Wallpaper->ClearJunk();
}

QString SpeedMenu::wallpaperGetCurJson() const
{
    std::u8string &&str = m_Wallpaper->GetJson();
    return QString::fromUtf8(reinterpret_cast<const char *>(str.data()), static_cast<int>(str.size()));
}

QString SpeedMenu::wallpaperGetCurWallpaper() const
{
    const std::u8string &str = m_Wallpaper->GetCurIamge().u8string();
    return QString::fromUtf8(reinterpret_cast<const char *>(str.data()), static_cast<int>(str.size()));
}

void SpeedMenu::wallpaperSetCurJson(const QString &str)
{
    QByteArray &&array = str.toUtf8();
    m_Wallpaper->SetJson(std::u8string(array.begin(), array.end()));
}

void SpeedMenu::wallpaperSetDrop(const QString &str)
{
    QByteArray &&array = str.toUtf8();
    YJson urls(array.begin(), array.end());
    std::deque<std::filesystem::path> paths;
    for (const auto &i : urls.getArray())
    {
        auto &temp = i.getValueString();
#ifdef _WIN32
        std::filesystem::path path(std::u8string(temp.begin() + 8, temp.end()));
#elif defined _linux_
        std::filesystem::path path(std::u8string(temp.begin() + 7, temp.end()));
#endif
        if (Wallpaper::IsImageFile(path))
            paths.emplace_front(std::move(path));
    }
    if (!paths.empty())
        m_Wallpaper->SetDropFile(std::move(paths));
}

void SpeedMenu::appShutdownComputer()
{
#ifdef WIN32
    ShellExecute(NULL, TEXT("open"), TEXT("shutdown"), TEXT("-s -t 0"), NULL, SW_HIDE);
#else
    QTimer::singleShot(500, std::bind(::system, "shutdown -h now"));
#endif
}

void SpeedMenu::appRestartComputer()
{
#ifdef WIN32
    ShellExecute(NULL, TEXT("open"), TEXT("shutdown"), TEXT("-r -t 0"), NULL, SW_HIDE);
#else
    QTimer::singleShot(500, std::bind(::system, "shutdown -r now"));
#endif
}

void SpeedMenu::appOpenDir(const QString &path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void SpeedMenu::appOpenAppDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath()));
}

void SpeedMenu::appOpenCfgDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
}

void SpeedMenu::wallpaperGetNext()
{
    m_Wallpaper->SetSlot(1);
}

void SpeedMenu::wallpaperGetPrev()
{
    m_Wallpaper->SetSlot(-1);
}

void SpeedMenu::wallpaperRemoveCurrent()
{
    m_Wallpaper->SetSlot(0);
}

bool SpeedMenu::toolOcrEnableScreenShotCut(const QString &keys, bool enable)
{
    QxtGlobalShortcut *shotCut = nullptr;
    if (enable)
    {
        if (shotCut)
            return true;
        QxtGlobalShortcut *shotCut = new QxtGlobalShortcut(this);
        if (shotCut->setShortcut(QKeySequence(keys)))
        {
            // connect(shotCut, &QxtGlobalShortcut::activated, this, &SpeedMenu::toolOcrGetScreenShotCut);
            return true;
        }
        else
        {
            std::cout << "Can't regist Shift+A\n";
        }
    }
    delete shotCut;
    shotCut = nullptr;
    return false;
}

QObject *SpeedMenu::toolOcrGrabScreen()
{
    PixmapContainer *pc = new PixmapContainer;
    pc->m_Image = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId()).toImage();
    QQmlEngine::setObjectOwnership(pc, QQmlEngine::JavaScriptOwnership);
    return pc;
}
