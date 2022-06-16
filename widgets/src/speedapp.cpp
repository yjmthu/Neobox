#include "speedapp.h"

#include <translater.h>
#include <wallpaper.h>
#include <yjson.h>
#include <appcode.hpp>

#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QBitmap>
#include <filesystem>

#include "speedbox.h"
#include "speedmenu.h"

VarBox* m_VarBox = nullptr;
extern std::unique_ptr<YJson> m_GlobalSetting;
extern const char* m_szClobalSettingFile;

inline void MessageBox(const std::u8string& msg) {
  QString str(
      QString::fromUtf8(reinterpret_cast<const char*>(msg.data()), 
      static_cast<int>(msg.size())));
  QMessageBox::information(nullptr, "Notice", str);
}

VarBox::VarBox() : QObject(), m_SpeedBox(nullptr) {
  m_VarBox = this;
  LoadFonts();
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  constexpr char relPath[] = ".config/Neobox";
  if (dir.exists(relPath) || dir.mkpath(relPath)) dir.cd(relPath);
  QDir::setCurrent(dir.absolutePath());
  auto picHome = QDir::toNativeSeparators(QStandardPaths::writableLocation(
                                              QStandardPaths::PicturesLocation))
                     .toUtf8();
  GetSetting();
  m_Tray = new QSystemTrayIcon;
  m_Timer = new QTimer;
  m_Timer->setSingleShot(true);
  m_Wallpaper = new Wallpaper(std::u8string(picHome.begin(), picHome.end()));
  GetSpeedBox();
  m_Menu = new SpeedMenu;  // Must before SpeedBox;
  m_Menu->showEvent(nullptr);
  m_Tray->setContextMenu(m_Menu);
  m_Tray->setIcon(QIcon(QStringLiteral(":/icons/speedbox.ico")));
  m_Tray->show();
  if (m_GlobalSetting->find(u8"FormGlobal")
          ->second[u8"ShowForm"]
          .second.isTrue())
    m_SpeedBox->show();
  m_Translater = new Translater;
  QObject::connect(m_Tray, &QSystemTrayIcon::activated, m_Translater,
                   &Translater::IntelligentShow);
}

VarBox::~VarBox() {
  delete m_Translater;
  delete m_Menu;
  delete m_SpeedBox;
  delete m_Wallpaper;
  delete m_Tray;
}

void VarBox::GetSetting() {
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


void VarBox::GetSpeedBox()
{
  qmlRegisterType<SpeedBox>("MySpeedBox", 1, 0, "SpeedBox");
  m_SpeedBox = new QQuickView;
  m_SpeedBox->rootContext()->setContextProperty("mainwindow", m_SpeedBox);
  m_SpeedBox->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
  m_SpeedBox->setColor(QColor(Qt::transparent));
  m_SpeedBox->setResizeMode(QQuickView::SizeRootObjectToView);
  m_SpeedBox->setSource(QUrl(QStringLiteral("qrc:/qmls/FloatingWindow.qml")));
  // m_SpeedBox->setSource(QUrl(QStringLiteral("FloatingWindow.qml")));
  m_SpeedBox->setCursor(Qt::PointingHandCursor);
  connect(m_SpeedBox->engine(), &QQmlEngine::quit, this, [](){
      qApp->exit(static_cast<int>(
                     ExitCode::RETCODE_RESTART));
  });
  // m_SpeedBox->setAcceptDrops(true);
  
  m_SpeedBox->show();

}
