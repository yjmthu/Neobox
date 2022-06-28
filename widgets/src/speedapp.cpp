#include "speedapp.h"

#include <translater.h>
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

#include "speedbox.h"
#include "speedmenu.h"

VarBox* m_VarBox = nullptr;
extern std::unique_ptr<YJson> m_GlobalSetting;
extern const char* m_szClobalSettingFile;

inline void MessageBox(const std::u8string& msg) {
  QString str(QString::fromUtf8(reinterpret_cast<const char*>(msg.data()),
                                static_cast<int>(msg.size())));
  QMessageBox::information(nullptr, "Notice", str);
}

VarBox::VarBox() : QObject() {
  m_VarBox = this;
  LoadFonts();
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  constexpr char relPath[] = ".config/Neobox";
  if (dir.exists(relPath) || dir.mkpath(relPath)) dir.cd(relPath);
  QDir::setCurrent(dir.absolutePath());
  LoadSettings();
  LoadQmlFiles();
}

VarBox::~VarBox() {}

void VarBox::LoadSettings() {
  if (!std::filesystem::exists(m_szClobalSettingFile)) {
    QFile::copy(":/jsons/Setting.json", m_szClobalSettingFile);
    QFile::setPermissions(m_szClobalSettingFile,
                          QFileDevice::ReadUser | QFileDevice::WriteUser);
  }
  m_GlobalSetting = std::make_unique<YJson>(m_szClobalSettingFile, YJson::UTF8);
  if (!std::filesystem::exists(Wallpaper::m_szWallScript)) {
    QFile::copy(":/scripts/SetWallpaper.sh", Wallpaper::m_szWallScript);
    QFile::setPermissions(
        Wallpaper::m_szWallScript,
        QFileDevice::ReadUser | QFileDevice::Permission::ExeUser);
  }
}

void VarBox::LoadFonts() {
  QFontDatabase::addApplicationFont(
      QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
  QFontDatabase::addApplicationFont(
      QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}

void VarBox::LoadQmlFiles() {
  QString prefix(":/");
  if (!std::filesystem::exists("qmls")) {
    std::filesystem::create_directory("qmls");
  }
  auto lst = {"qmls/FloatingWindow.qml", "qmls/MainMenu.qml",
              "qmls/NeoMenuItem.qml", "qmls/SystemTray.qml"};
  for (const auto& i : lst) {
    if (!QFile::exists(i)) {
      QFile::copy(prefix + i, i);
      QFile::setPermissions(i, QFileDevice::ReadUser);
    }
  }
}

#if 0
void VarBox::GetSpeedBox() {
  qmlRegisterType<SpeedBox>("Neobox", 1, 0, "SpeedBox");
  qmlRegisterType<SpeedMenu>("Neobox", 1, 0, "SpeedMenu");
  m_SpeedBox = new QQuickView;
  m_SpeedBox->rootContext()->setContextProperty("mainwindow", m_SpeedBox);
  // m_SpeedBox->setAcceptDrops(true);

  m_SpeedBox->show();
}
#endif
