#include <qxtglobalshortcut.h>
#include <screenfetch.h>
#include <speedapp.h>
#include <speedbox.h>
#include <speedmenu.h>
#include <sysapi.h>
#include <text.h>
#include <wallpaper.h>
#include <yjson.h>

#include <QActionGroup>
#include <QColorDialog>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QtQuick>
#include <appcode.hpp>
#include <regex>
#include <thread>

// #include <KWindowEffects>

extern std::unique_ptr<YJson> m_GlobalSetting;
extern const char* m_szClobalSettingFile;

using namespace std::literals;

SpeedMenu::SpeedMenu()
  : QObject(nullptr)
{
  auto picHome = QDir::toNativeSeparators(QStandardPaths::writableLocation(
                                            QStandardPaths::PicturesLocation))
                   .toUtf8();
  m_Wallpaper = new Wallpaper(std::u8string(picHome.begin(), picHome.end()));
}

SpeedMenu::~SpeedMenu()
{
  delete m_Wallpaper;
}

bool
SpeedMenu::appAutoStart()
{
#ifdef WIN32
  return
      !QSettings(QStringLiteral(
                     "HKEY_CURRENT_"
                     "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
                 QSettings::NativeFormat)
           .value("Neobox")
           .toString()
           .compare(QDir::toNativeSeparators(qApp->applicationFilePath())));
#else
  QString m_AutoStartFile =
    QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
    "/.config/autostart/Neobox.desktop";
  if (!QFile::exists(m_AutoStartFile)) {
    return false;
  } else {
    QSettings m_Setting(m_AutoStartFile, QSettings::IniFormat);
    m_Setting.beginGroup("Desktop Entry");
    QString str = m_Setting.value("Exec", "null").toString();
    m_Setting.endGroup();
    return str.indexOf(qApp->applicationFilePath()) != -1;
  }
#endif
}

void
SpeedMenu::appSetAutoStart(bool start)
{
#ifdef WIN32
  QSettings reg(
    QStringLiteral("HKEY_CURRENT_"
                   "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
    QSettings::NativeFormat);
  if (checked)
    reg.setValue("Neobox",
                 QDir::toNativeSeparators(qApp->applicationFilePath()));
  else
    reg.remove("Neobox");
#else
  QString configLocation =
    QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
  std::filesystem::path desktopFolderName =
    (configLocation + "/autostart").toStdU16String();
  if (!std::filesystem::exists(desktopFolderName))
    std::filesystem::create_directory(desktopFolderName);
  QString iconFileName = configLocation + "/Neobox/Neobox.ico";
  if (start) {
    if (!QFile::exists(iconFileName)) {
      QFile::copy(":/icons/speedbox.ico", iconFileName);
      QFile::setPermissions(iconFileName, QFileDevice::ReadUser);
    }
    std::ofstream file(desktopFolderName / "Neobox.desktop"s,
                       std::ios::binary | std::ios::out);
    file << "[Desktop Entry]"sv << std::endl
         << "Name=Neobox" << std::endl
         << "Comment=A powerful wallpaper tool." << std::endl
         << "Icon=" << iconFileName.toStdString() << std::endl
         << "Exec=sh -c \"(sleep 3 && exec \'"
         << qApp->applicationFilePath().toStdString() << "\' -b)\"" << std::endl
         << "Categories=System" << std::endl
         << "Terminal=false" << std::endl
         << "Type=Application" << std::endl;
    file.close();
  } else if (std::filesystem::exists(desktopFolderName /= "Neobox.desktop"s)) {
    std::filesystem::remove(desktopFolderName);
  }
#endif // WIN32
  emit appAutoStartChanged();
}

int
SpeedMenu::wallpaperType() const
{
  return m_Wallpaper->GetImageType();
}

void
SpeedMenu::wallpaperSetType(int type)
{
  if (type == m_Wallpaper->GetImageType())
    return;
  emit wallpaperTypeChanged(m_Wallpaper->SetImageType(type));
}

int
SpeedMenu::wallpaperTimeInterval() const
{
  return m_Wallpaper->GetTimeInterval();
}

void
SpeedMenu::wallpaperSetTimeInterval(int minute)
{
  m_Wallpaper->SetTimeInterval(minute);
  emit wallpaperTimeIntervalChanged();
}

QString
SpeedMenu::wallpaperDir() const
{
  return QString::fromStdU16String(m_Wallpaper->GetImageDir().u16string());
}

void
SpeedMenu::wallpaperSetDir(const QString& str)
{
  m_Wallpaper->SetCurDir(str.toStdU16String());
  emit wallpaperDirChanged();
}

bool
SpeedMenu::wallpaperAutoChange() const
{
  return m_Wallpaper->GetAutoChange();
}

void
SpeedMenu::wallpaperSetAutoChange(bool val)
{
  m_Wallpaper->SetAutoChange(val);
  emit wallpaperAutoChangeChanged();
}

bool
SpeedMenu::wallpaperFirstChange() const
{
  return m_Wallpaper->GetFirstCHange();
}

void
SpeedMenu::wallpaperSetFirstChange(bool val)
{
  m_Wallpaper->SetFirstChange(val);
  emit wallpaperFirstChangeChanged();
}

void
SpeedMenu::wallpaperUndoDelete()
{
  m_Wallpaper->UndoDelete();
}

void
SpeedMenu::wallpaperClearJunk()
{
  m_Wallpaper->ClearJunk();
}

QString
SpeedMenu::wallpaperGetCurJson() const
{
  std::u8string&& str = m_Wallpaper->GetJson();
  return QString::fromUtf8(reinterpret_cast<const char*>(str.data()),
                           str.size());
}

void
SpeedMenu::wallpaperSetCurJson(const QString& str)
{
  QByteArray&& array = str.toUtf8();
  m_Wallpaper->SetJson(std::u8string(array.begin(), array.end()));
}

void
SpeedMenu::wallpaperSetDrop(const QString& str)
{
  QByteArray&& array = str.toUtf8();
  YJson urls(array.begin(), array.end());
  std::deque<std::filesystem::path> paths;
  for (const auto& i : urls.getArray()) {
    auto& temp = i.getValueString();
    std::filesystem::path path(std::u8string(temp.begin() + 7, temp.end()));
    if (Wallpaper::IsImageFile(path))
      paths.emplace_front(std::move(path));
  }
  if (!paths.empty())
    m_Wallpaper->SetDropFile(std::move(paths));
}

void
SpeedMenu::toolOcrGetScreenShotCut()
{
  ScreenFetch getscreen;
  getscreen.exec();
  void* image = getscreen.m_Picture;
  if (image)
    (new TextDlg(image))->show();
}

void
SpeedMenu::appShutdownComputer()
{
#ifdef WIN32
  QTimer::singleShot(500, std::bind(::system, "shutdown -s -t 0"));
#else
  QTimer::singleShot(500, std::bind(::system, "shutdown -h now"));
#endif
}

void
SpeedMenu::appRestartComputer()
{
#ifdef WIN32
  QTimer::singleShot(500, std::bind(::system, "shutdown -r -t 0"));
#else
  QTimer::singleShot(500, std::bind(::system, "shutdown -r now"));
#endif
}

void
SpeedMenu::appOpenDir(const QString& path)
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void
SpeedMenu::appOpenAppDir()
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath()));
}

void
SpeedMenu::appOpenCfgDir()
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
}

void
SpeedMenu::wallpaperGetNext()
{
  m_Wallpaper->SetSlot(1);
}

void
SpeedMenu::wallpaperGetPrev()
{
  m_Wallpaper->SetSlot(-1);
}

void
SpeedMenu::wallpaperRemoveCurrent()
{
  m_Wallpaper->SetSlot(0);
}

bool
SpeedMenu::toolOcrEnableScreenShotCut(const QString& keys, bool enable)
{
  QxtGlobalShortcut* shotCut = nullptr;
  if (enable) {
    if (shotCut)
      return true;
    QxtGlobalShortcut* shotCut = new QxtGlobalShortcut(this);
    if (shotCut->setShortcut(QKeySequence(keys))) {
      connect(shotCut,
              &QxtGlobalShortcut::activated,
              this,
              &SpeedMenu::toolOcrGetScreenShotCut);
      return true;
    } else {
      std::cout << "Can't regist Shift+A\n";
    }
  }
  delete shotCut;
  shotCut = nullptr;
  return false;
}

#if 0

void SpeedMenu::SetupSettingMenu() {
  auto& m_Setting = m_GlobalSetting->find(u8"Wallpaper")->second;
  QMenu* m_pSettingMenu = new QMenu(this);
  QMenu* m_pWallpaperMenu = new QMenu(m_pSettingMenu);
  QMenu* m_pToolsMenu = new QMenu(m_pSettingMenu);
  QMenu* m_pAppSettingMenu = new QMenu(m_pSettingMenu);
  QMenu* m_pAboutMenu = new QMenu(m_pSettingMenu);
  auto ptr = m_pSettingMenu->addAction("软件设置");
  ptr->setMenu(m_pAppSettingMenu);

  ptr = m_pSettingMenu->addAction("壁纸设置");
  ptr->setMenu(m_pWallpaperMenu);
  ptr = m_pWallpaperMenu->addAction("类型选择");
  SetupImageType(m_pWallpaperMenu, ptr);
  ptr = m_pWallpaperMenu->addAction("首次更换");
  ptr->setCheckable(true);
  ptr->setChecked(m_Setting.find(u8"FirstChange")->second.isTrue());
  connect(ptr, &QAction::triggered,
          std::bind(&Wallpaper::SetFirstChange, m_VarBox->m_Wallpaper,
                    std::placeholders::_1));
  ptr = m_pWallpaperMenu->addAction("自动更换");
  ptr->setCheckable(true);
  ptr->setChecked(m_Setting.find(u8"AutoChange")->second.isTrue());
  connect(ptr, &QAction::triggered,
          std::bind(&Wallpaper::SetAutoChange, m_VarBox->m_Wallpaper,
                    std::placeholders::_1));

  m_pSettingMenu->addAction("工具设置")->setMenu(m_pToolsMenu);
  m_pToolsMenu->addAction("截图识字");
  m_pToolsMenu->addAction("单词翻译");
  m_pToolsMenu->addAction("句段翻译");
  // connect(m_pToolsMenu->addAction("测试接口"), &QAction::triggered, this, [](){
  //     QDialog dlg;
  //     dlg.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  //     dlg.setAttribute(Qt::WA_TranslucentBackground);
  //     QFrame frame(&dlg);
  //     frame.setGeometry(0,0,dlg.width(), dlg.height());
  //     dlg.setStyleSheet("background-color:rgba(100,100,100,100);");
  //     KWindowEffects::enableBlurBehind(dlg.winId());
  //     dlg.exec();
  // });

  ptr = m_pSettingMenu->addAction("关于软件");
  ptr->setMenu(m_pAboutMenu);
  m_pAboutMenu->addAction("版本信息");
  m_pAboutMenu->addAction("检查更新");

  m_pSettingMenu->addSeparator();
  connect(
      m_pWallpaperMenu->addAction("打开路径"), &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdU16String(
            m_VarBox->m_Wallpaper->GetImageDir().u16string())));
      });
  connect(m_pWallpaperMenu->addAction("设置路径"), &QAction::triggered, this,
          []() {
            auto i = QFileDialog::getExistingDirectory(
                nullptr, "选择文件夹",
                QString::fromStdU16String(
                    m_VarBox->m_Wallpaper->GetImageDir().u16string()),
                QFileDialog::ShowDirsOnly);
            if (!i.isEmpty()) {
              m_VarBox->m_Wallpaper->SetCurDir(i.toStdU16String());
            }
          });
  connect(m_pWallpaperMenu->addAction("时间间隔"), &QAction::triggered, this,
          []() {
            int val =
                QInputDialog::getInt(nullptr, "输入时间", "时间间隔（分钟）：",
                                     m_VarBox->m_Wallpaper->GetTimeInterval(),
                                     5, std::numeric_limits<int>::max());
            m_VarBox->m_Wallpaper->SetTimeInterval(val);
          });
  m_AdditionalAction = m_pWallpaperMenu->addAction("附加设置");
  m_Actions->setMenu(m_pSettingMenu);
  SetAdditionalMenu();

  m_AutoStartApp = m_pAppSettingMenu->addAction("开机自启");
  m_AutoStartApp->setCheckable(true);
  ptr = m_pAppSettingMenu->addAction("显示界面");
  ptr->setCheckable(true);
  ptr->setChecked(m_GlobalSetting->find(u8"FormGlobal")
                      ->second[u8"ShowForm"]
                      .second.isTrue());
  connect(ptr, &QAction::triggered, this, [](bool checked) {
    m_GlobalSetting->find(u8"FormGlobal")->second[u8"ShowForm"].second =
        checked;
    if (checked) {
      m_VarBox->m_SpeedBox->show();
    } else {
      m_VarBox->m_SpeedBox->hide();
    }
    m_GlobalSetting->toFile(m_szClobalSettingFile);
  });
  ptr = m_pAppSettingMenu->addAction("修改界面");
  QMenu* m_Menu = new QMenu(this);
  ptr->setMenu(m_Menu);
  QActionGroup* m_Group = new QActionGroup(m_Menu);
  m_Group->setExclusive(true);
  const char* m_Strs[] {"存储占用", "上传速度", "下载速度"};
  for (int i = 0, j = m_VarBox->m_SpeedBox->findChild<QObject*>("rect")->property("textindex").toInt(); i < 3; ++i) {
    ptr = m_Menu->addAction(m_Strs[i]);
    ptr->setCheckable(true);
    ptr->setChecked(i == j);
    m_Group->addAction(ptr);
    connect(ptr, &QAction::triggered, this, [i]() { m_VarBox->m_SpeedBox->findChild<QObject*>("rect")->setProperty("textindex", i); });
  }
  m_Menu->addSeparator();
  static const char* li[] = {"MemUseage", "NetUpSpeed", "NetDownSpeed"};
  connect(m_Menu->addAction("修改颜色"), &QAction::triggered, this, []() {
    auto ptr = m_VarBox->m_SpeedBox->findChild<QObject*>("rect");
    int index = ptr->property("textindex").toInt();
    ptr = ptr->findChild<QObject*>(li[index]);
    QColor col(ptr->property("color").toString());
    int alpha = col.alpha();
    col = QColorDialog::getColor(col);
    if (col.isValid()) {
      col.setAlpha(alpha);
      ptr->setProperty("color", col.name(QColor::HexArgb));
    }
  });
  connect(m_Menu->addAction("修改字体"), &QAction::triggered, this, []() {
    auto ptr = m_VarBox->m_SpeedBox->findChild<QObject*>("rect");
    int index = ptr->property("textindex").toInt();
    ptr = ptr->findChild<QObject*>(li[index]);
    bool ok;
    auto _font = QFontDialog::getFont(&ok, ptr->property("font").value<QFont>());
    if (ok) ptr->setProperty("font", _font);
  });

  connect(m_pAppSettingMenu->addAction("更新菜单"), &QAction::triggered, this,
          &SpeedMenu::UpdateStyle);
  connect(m_pAppSettingMenu->addAction("还原位置"), &QAction::triggered, this,
          []() {
            m_VarBox->m_SpeedBox->setPosition(100, 100);
          });
  connect(m_pAppSettingMenu->addAction("重启软件"), &QAction::triggered, this,
          []() { qApp->exit(static_cast<int>(ExitCode::RETCODE_RESTART)); });
  connect(m_pAppSettingMenu->addAction("背景颜色"), &QAction::triggered, this, []() {
    auto ptr = m_VarBox->m_SpeedBox->findChild<QObject*>("rect");
    QColor col(ptr->property("color").toString());
    int alpha = col.alpha();
    col = QColorDialog::getColor(col, nullptr, "选择颜色");
    if (col.isValid()) {
      col.setAlpha(alpha);
      ptr->setProperty("color", col.name(QColor::HexArgb));
    }
  });
  connect(m_pAppSettingMenu->addAction("背景透明"), &QAction::triggered, this, []() {
    auto ptr = m_VarBox->m_SpeedBox->findChild<QObject*>("rect");
    QColor col = QColor(ptr->property("color").toString());
    col.setAlpha(
        QInputDialog::getInt(nullptr, QStringLiteral("请输入不透明度"),
                             QStringLiteral("整数(1~255)"),
                             col.alpha(), 0, 255));
    ptr->setProperty("color", col.name(QColor::HexArgb));
  });
  ptr = m_pAppSettingMenu->addAction("背景模糊");
  ptr->setCheckable(true);
  ptr->setChecked(m_VarBox->m_SpeedBox->findChild<QObject*>("settings")->property("blur").toBool());
  connect(ptr, &QAction::triggered, this, [](bool checked) {
    m_VarBox->m_SpeedBox->findChild<QObject*>("settings")->setProperty("blur", checked);
  });
}

void SpeedMenu::SetupImageType(QMenu* parent, QAction* ac) {
  int index = 0;
  static int type = m_GlobalSetting->find(u8"Wallpaper")
                        ->second.find(u8"ImageType")
                        ->second.getValueInt();
  QMenu* mu = new QMenu(parent);

  auto acGroup = new QActionGroup(this);
  acGroup->setExclusive(true);
  for (const auto& i :
       {"壁纸天堂", "必应壁纸", "直链网址", "本地壁纸", "脚本输出"}) {
    auto ptr = mu->addAction(i);
    ptr->setCheckable(true);
    acGroup->addAction(ptr);
    ptr->setChecked(index == type);
    connect(ptr, &QAction::triggered, this, [index, this]() {
      if (m_VarBox->m_Wallpaper->IsWorking() &&
          QMessageBox::question(nullptr, QStringLiteral("提示"),
                                QStringLiteral("当前正忙，是否强制继续？")) !=
              QMessageBox::Yes) {
        return;
      }
      m_VarBox->m_Wallpaper->SetImageType(index);
      SetAdditionalMenu();
    });
    ++index;
  }
  ac->setMenu(mu);
}

void SpeedMenu::SetAdditionalMenu() {
  static QMenu* m_Menus = nullptr;
  QMenu* m_tempMenu = new QMenu(this);
  m_AdditionalAction->setMenu(m_tempMenu);
  if (m_Menus) {
    delete m_Menus;
    m_Menus = m_tempMenu;
  }
  switch (m_VarBox->m_Wallpaper->GetImageType()) {
    case 0: {
      QActionGroup* group = new QActionGroup(m_tempMenu);
      YJson* m_Setting = *reinterpret_cast<YJson* const*>(
          m_VarBox->m_Wallpaper->GetDataByName("m_Setting"));
      const auto& curType =
          m_Setting->find(u8"WallhavenCurrent")->second.getValueString();
      m_tempMenu->addAction("壁纸类型");
      m_tempMenu->addSeparator();
      auto& _jjk = m_Setting->find(u8"WallhavenApi")->second.getObject();
      for (auto i = _jjk.begin(); i != _jjk.end(); ++i) {
        const std::u8string_view temp = i->first;
        auto ac = m_tempMenu->addAction(
            QString::fromUtf8(reinterpret_cast<const char*>(i->first.data()),
                              static_cast<int>(i->first.size())));
        ac->setCheckable(true);
        ac->setChecked(i->first == curType);
        group->addAction(ac);
        QMenu* mn = new QMenu(m_tempMenu);
        ac->setMenu(mn);
        connect(mn->addAction("启用此项"), &QAction::triggered, this,
                [temp, m_Setting, ac]() {
                  if (ac->isChecked()) {
                    QMessageBox::critical(
                        nullptr, QStringLiteral("警告"),
                        QStringLiteral("当前正在使用该类型！"));
                    return;
                  }
                  ac->setChecked(true);
                  int val = QInputDialog::getInt(
                      nullptr, QStringLiteral("输入页面"),
                      QStringLiteral("页面位置（1~100）："),
                      m_Setting->find(u8"PageNumber")->second.getValueInt(), 1,
                      100);
                  m_Setting->find(u8"WallhavenCurrent")->second.setText(temp);
                  m_Setting->find(u8"PageNumber")->second.setValue(val);
                  m_VarBox->m_Wallpaper->SetSlot(3);
                });
        if (i->second.find(u8"Parameter")->second.isObject()) {
          connect(
              mn->addAction("参数设置"), &QAction::triggered, this,
              [i, this, ac, m_Setting]() {
                QDialog dlg;
                QVBoxLayout* vlayout = new QVBoxLayout(&dlg);
                QHBoxLayout hlayout1, hlayout2;
                QLineEdit* lineEdit = new QLineEdit(&dlg);
                auto label = new QLabel(&dlg);
                label->setText(QStringLiteral("类型名称"));
                lineEdit->setText(QString::fromUtf8(
                    reinterpret_cast<const char*>(i->first.data()),
                    static_cast<int>(i->first.size())));
                hlayout1.addWidget(label);
                hlayout1.addWidget(lineEdit);
                dlg.setLayout(vlayout);
                int row = i->second.find(u8"Parameter")->second.sizeO(),
                    index = 0;
                QTableWidget m_TableWidget(row, 2);
                vlayout->addLayout(&hlayout1);
                vlayout->addWidget(&m_TableWidget);
                vlayout->addLayout(&hlayout2);
                dlg.setWindowTitle(QStringLiteral("参数设置"));
                m_TableWidget.setHorizontalHeaderLabels(
                    QStringList{"key", "value"});
                QPushButton* bt1 = new QPushButton(&dlg);
                bt1->setText(QStringLiteral("添加"));
                QPushButton* bt2 = new QPushButton(&dlg);
                bt2->setText(QStringLiteral("删除"));
                QPushButton* bt3 = new QPushButton(&dlg);
                bt3->setText(QStringLiteral("确认"));
                hlayout2.addWidget(bt1);
                hlayout2.addWidget(bt2);
                hlayout2.addWidget(bt3);
                connect(bt1, &QPushButton::clicked, this, [&m_TableWidget]() {
                  m_TableWidget.setRowCount(m_TableWidget.rowCount() + 1);
                });
                connect(bt2, &QPushButton::clicked, this, [&m_TableWidget]() {
                  m_TableWidget.removeRow(m_TableWidget.currentRow());
                });
                connect(
                    bt3, &QPushButton::clicked, this,
                    [&m_TableWidget, this, &dlg, &i, ac, lineEdit,
                     m_Setting]() {
                      int row = m_TableWidget.rowCount();
                      YJson js(YJson::Object);
                      for (int i = 0; i < row; ++i) {
                        QByteArray _tmp =
                            m_TableWidget.item(i, 0)->text().toUtf8();
                        std::u8string key(_tmp.begin(), _tmp.end());
                        _tmp = m_TableWidget.item(i, 1)->text().toUtf8();
                        std::u8string val(_tmp.begin(), _tmp.end());
                        if (key.empty() || val.empty()) {
                          continue;
                        } else {
                          js.append(val, key);
                        }
                      }
                      if (!js.emptyO()) {
                        auto array = lineEdit->text().toUtf8();
                        std::u8string newKey(array.begin(), array.end());
                        if (newKey.empty()) newKey = u8"新类型";
                        YJson::swap(i->second.find(u8"Parameter")->second, js);
                        if (newKey != i->first) {
                          i->first = newKey;
                          m_Setting->find(u8"WallhavenCurrent")
                              ->second.setText(newKey);
                        }
                        if (m_VarBox->m_Wallpaper->IsWorking()) return;
                        m_VarBox->m_Wallpaper->SetSlot(ac->isChecked() ? 3 : 2);
                        SetupSettingMenu();
                      }
                      dlg.close();
                    });
                for (auto& j :
                     i->second.find(u8"Parameter")->second.getObject()) {
                  m_TableWidget.setItem(
                      index, 0,
                      new QTableWidgetItem(QString::fromUtf8(
                          reinterpret_cast<const char*>(j.first.data()),
                          static_cast<int>(j.first.size()))));
                  if (j.second.isNumber()) {
                    m_TableWidget.setItem(index++, 1,
                                          new QTableWidgetItem(QString::number(
                                              j.second.getValueInt())));
                  } else {
                    std::u8string_view str = j.second.getValueString();
                    m_TableWidget.setItem(
                        index++, 1,
                        new QTableWidgetItem(QString::fromUtf8(
                            reinterpret_cast<const char*>(str.data()),
                            static_cast<int>(str.size()))));
                  }
                }
                dlg.exec();
              });
        }
        if (!ac->isChecked()) {
          connect(mn->addAction("删除此项"), &QAction::triggered, this,
                  [m_Setting, i, this]() {
                    m_Setting->find(u8"WallhavenApi")->second.remove(i);
                    m_VarBox->m_Wallpaper->SetSlot(2);
                    SetupSettingMenu();
                  });
        }
      }
      connect(
          m_tempMenu->addAction("添加更多"), &QAction::triggered, this,
          [this, m_Setting]() {
            QString m_TypeName =
                QInputDialog::getText(nullptr, QStringLiteral("获取名称"),
                                      QStringLiteral("请输入名称："));
            if (m_TypeName.isEmpty()) return;
            auto m_TypeDir =
                QFileDialog::getExistingDirectory(
                    nullptr, "选择文件夹",
                    QString::fromStdU16String(
                        m_VarBox->m_Wallpaper->GetImageDir().u16string()),
                    QFileDialog::ShowDirsOnly)
                    .toStdString();
            if (m_TypeDir.empty()) return;
            QDialog dlg(nullptr);
            QVBoxLayout* vlayout = new QVBoxLayout(&dlg);
            QHBoxLayout hlayout;
            dlg.setLayout(vlayout);
            QTableWidget m_TableWidget(0, 2);
            vlayout->addWidget(&m_TableWidget);
            dlg.setWindowTitle(QStringLiteral("参数设置"));
            m_TableWidget.setHorizontalHeaderLabels(
                QStringList{"key", "value"});
            // m_TableWidget.setSelectionBehavior (
            // QAbstractItemView::SelectRows);
            QPushButton* bt1 = new QPushButton(&dlg);
            bt1->setText(QStringLiteral("添加"));
            QPushButton* bt2 = new QPushButton(&dlg);
            bt2->setText(QStringLiteral("删除"));
            QPushButton* bt3 = new QPushButton(&dlg);
            bt3->setText(QStringLiteral("确认"));
            connect(bt1, &QPushButton::clicked, this, [&m_TableWidget]() {
              m_TableWidget.setRowCount(m_TableWidget.rowCount() + 1);
            });
            connect(bt2, &QPushButton::clicked, this, [&m_TableWidget]() {
              m_TableWidget.removeRow(m_TableWidget.currentRow());
            });
            connect(
                bt3, &QPushButton::clicked, this,
                [&m_TableWidget, m_Setting, this, &m_TypeDir, &m_TypeName,
                 &dlg]() {
                  int row = m_TableWidget.rowCount();
                  YJson js(YJson::Object);
                  js.append(m_TypeDir, u8"Directory");
                  auto& ptr = js.append(YJson::Object, u8"Parameter")->second;
                  for (int i = 0; i < row; ++i) {
                    std::u8string&& key =
                        GetU8String(m_TableWidget.item(i, 0)->text().toUtf8());
                    std::u8string&& val =
                        GetU8String(m_TableWidget.item(i, 1)->text().toUtf8());
                    if (key.empty() || val.empty()) {
                      continue;
                    } else {
                      ptr.append(val, key);
                    }
                  }
                  if (!ptr.emptyO()) {
                    YJson::swap(m_Setting->find(u8"WallhavenApi")
                                    ->second
                                    .append(YJson::Object,
                                            GetU8String(m_TypeName.toUtf8()))
                                    ->second,
                                js);
                    m_VarBox->m_Wallpaper->SetSlot(3);
                    SetupSettingMenu();
                  }
                  dlg.close();
                });
            hlayout.addWidget(bt1);
            hlayout.addWidget(bt2);
            hlayout.addWidget(bt3);
            vlayout->addLayout(&hlayout);
            dlg.exec();
          });
      m_tempMenu->addSeparator();
      connect(m_tempMenu->addAction("壁纸网站"), &QAction::triggered, this,
              []() {
                const auto& curImage =
                    m_VarBox->m_Wallpaper->GetCurIamge().string();
                const std::regex pattern(
                    "^.*?wallhaven-([a-z0-9]{6})\\.(jpg|png)$"s);
                std::smatch _result;
                if (std::regex_match(curImage, _result, pattern)) {
                  QDesktopServices::openUrl(QString::fromStdString(
                      "https://wallhaven.cc/w/" + _result.str(1)));
                } else {
                  QMessageBox::critical(nullptr, "错误",
                                        "当前壁纸并不是Wallhaven壁纸！");
                }
              });
      connect(
          m_tempMenu->addAction("相似壁纸"), &QAction::triggered, this, []() {
            const auto& curImage =
                m_VarBox->m_Wallpaper->GetCurIamge().string();
            std::cout << curImage << std::endl;
            const std::regex pattern(
                "^.*?wallhaven-([a-z0-9]{6})\\.(jpg|png)$"s);
            std::smatch _result;
            if (std::regex_match(curImage, _result, pattern)) {
              QDesktopServices::openUrl(QString::fromStdString(
                  "https://wallhaven.cc/search?q=like%3A" + _result.str(1)));
            } else {
              QMessageBox::critical(nullptr, "错误",
                                    "当前壁纸并不是Wallhaven壁纸！");
            }
          });
      break;
    }
    case 1: {
      auto* ptr = m_tempMenu->addAction("自动下载");
      ptr->setCheckable(true);
      ptr->setChecked(true);
      ptr = m_tempMenu->addAction("选择地区");
      connect(ptr, &QAction::triggered, this, []() {
        std::u8string& str =
            (*reinterpret_cast<YJson* const*>(
                 m_VarBox->m_Wallpaper->GetDataByName("m_Setting")))
                ->find(u8"mkt")
                ->second.getValueString();
        QByteArray array(
            QInputDialog::getText(
                nullptr, "请输入地区", "地区名称：", QLineEdit::Normal,
                QString::fromUtf8(reinterpret_cast<const char*>(str.data()),
                                  static_cast<int>(str.size())))
                .toUtf8());
        if (std::equal(str.begin(), str.end(), array.begin(), array.end())) {
          QMessageBox::information(nullptr, "出错", "并未修改字符串！");
          return;
        }
        if (!array.isEmpty()) {
          str.assign(array.begin(), array.end());
          m_VarBox->m_Wallpaper->SetSlot(2);
        }
      });
      ptr = m_tempMenu->addAction("壁纸信息");
      connect(ptr, &QAction::triggered, this, []() {
        YJson& m_Setting = **reinterpret_cast<YJson* const*>(
            m_VarBox->m_Wallpaper->GetDataByName("m_Setting"));
        std::u8string_view _link =
            m_Setting[u8"images"]
                .second[m_Setting.find(u8"index")->second.getValueInt()]
                       [u8"copyrightlink"]
                .second.getValueString();
        QDesktopServices::openUrl(
            QString::fromUtf8(reinterpret_cast<const char*>(_link.data()),
                              static_cast<int>(_link.size())));
      });
      break;
    }
    case 2: {
      QActionGroup* group = new QActionGroup(m_tempMenu);
      YJson* m_Setting = *reinterpret_cast<YJson* const*>(
          m_VarBox->m_Wallpaper->GetDataByName("m_Setting"));
      [[maybe_unused]] const auto& curType =
          m_Setting->find(u8"WallhavenCurrent")->second.getValueString();
      m_tempMenu->addAction("壁纸类型");
      m_tempMenu->addSeparator();
      std::u8string_view str =
          m_Setting->find(u8"ApiUrl"sv)->second.getValueString();
      for (auto& i : m_Setting->find(u8"ApiData"sv)->second.getObject()) {
        auto ptr = m_tempMenu->addAction(
            QString::fromUtf8(reinterpret_cast<const char*>(i.first.data()),
                              static_cast<int>(i.first.size())));
        ptr->setCheckable(true);
        group->addAction(ptr);
        if (str == i.first) ptr->setChecked(true);
        connect(ptr, &QAction::triggered, this, [m_Setting, ptr]() {
          QByteArray array(ptr->text().toUtf8());
          std::u8string temp(array.begin(), array.end());
          std::u8string& cur =
              m_Setting->find(u8"ApiUrl")->second.getValueString();
          if (cur != temp) {
            if (m_VarBox->m_Wallpaper->IsWorking()) {
              QMessageBox::warning(nullptr, "出错", "当前正忙！");
              return;
            }
            cur.assign(std::move(temp));
            m_VarBox->m_Wallpaper->SetSlot(2);
          } else {
            QMessageBox::information(nullptr, "提示", "当前已经是该类型！");
          }
        });
      }
      break;
    }
    case 3:
    case 4:
    default:
      break;
  }
}
#endif
