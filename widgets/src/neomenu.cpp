#include <neomenu.h>
#include <sysapi.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>

#include <map>

#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QStandardPaths>
#include <QUrl>

#ifdef _WIN32
#include <Windows.h>
#elif def __linux__
#else
#endif

NeoMenu::NeoMenu(QWidget* parent) : QMenu(parent), m_Wallpaper(new Wallpaper) {
  // setWindowFlags(Qt::FramelessWindowHint);
  // setAttribute(Qt::WA_TranslucentBackground, true);
  InitWallpaper();
  InitFunctionMap();
  QFile fJson(QStringLiteral(":/jsons/menucontent.json"));
  if (!fJson.open(QIODevice::ReadOnly)) {
    qApp->quit();
    return;
  }
  QByteArray array = fJson.readAll();
  GetMenuContent(this, YJson(array.begin(), array.end()));
  fJson.close();
}

NeoMenu::~NeoMenu() {
  delete m_Wallpaper;
}

void NeoMenu::InitWallpaper() {
  auto& jsWallSetting = VarBox::GetSettings(u8"Wallpaper");
  m_Wallpaper->SetImageType(jsWallSetting[u8"ImageType"].second.getValueInt());
  m_Wallpaper->SetAutoChange(jsWallSetting[u8"AutoChange"].second.isTrue());
  m_Wallpaper->SetFirstChange(jsWallSetting[u8"FirstChange"].second.isTrue());
  m_Wallpaper->SetTimeInterval(
      jsWallSetting[u8"TimeInterval"].second.getValueInt());
}

void NeoMenu::InitFunctionMap() {
  m_FuncNormalMap = {
      {u8"SystemShutdown", std::bind(ShellExecuteW, nullptr, L"open",
                                     L"shutdown", L"-s -t 0", nullptr, 0)},
      {u8"SystemRestart", std::bind(ShellExecuteW, nullptr, L"open",
                                    L"shutdown", L"-r -t 0", nullptr, 0)},
      {u8"AppQuit", &QApplication::quit},
      {u8"AppOpenExeDir",
       std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(qApp->applicationDirPath()))},
      {u8"AppOpenWallpaperDir",
       [this]() {
         QDesktopServices::openUrl(QUrl::fromLocalFile(
             QString::fromStdWString(m_Wallpaper->GetImageDir().wstring())));
       }},
      {u8"AppOpenConfigDir",
       std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath()))},
      {u8"ToolOcrGetScreen", []() {}},
      {u8"WallpaperPrev", std::bind(&Wallpaper::SetPrevious, m_Wallpaper)},
      {u8"WallpaperNext", std::bind(&Wallpaper::SetNext, m_Wallpaper)},
      {u8"WallpaperDislike", std::bind(&Wallpaper::RemoveCurrent, m_Wallpaper)},
      {u8"WallpaperUndoDislike",
       std::bind(&Wallpaper::UndoDelete, m_Wallpaper)},
      {u8"WallpaperCollect", std::bind(&Wallpaper::SetFavorite, m_Wallpaper)},
      {u8"WallpaperUndoCollect",
       std::bind(&Wallpaper::UnSetFavorite, m_Wallpaper)},
      {u8"WallpaperDir",
       [this]() {
         QString current =
             QString::fromStdWString(m_Wallpaper->GetCurIamge().wstring());
         if (!ChooseFolder("选择壁纸文件夹", current))
           return;
         m_Wallpaper->SetCurDir(current.toStdWString());
       }},
      {u8"WallpaperTimeInterval",
       [this]() {
         int iOldTime = m_Wallpaper->GetTimeInterval();
         int iNewTime = QInputDialog::getInt(this, "输入时间间隔",
                                             "时间间隔（分钟）：", iOldTime, 5);
         if (iNewTime != iOldTime) {
           m_Wallpaper->SetTimeInterval(iNewTime);
           VarBox::GetSettings(u8"Wallpaper")[u8"TimeInterval"].second.setValue(
               iNewTime);
           VarBox::WriteSettings();
         }
       }},
      {u8"AppWbsite", std::bind(QDesktopServices::openUrl,
                                QUrl("https://www.github.com/yjmthu/Neobox"))}};

  m_FuncCheckMap = {
      {u8"AppAutoSatrt",
       {[](bool checked) {
          wchar_t pPath[] =
              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
          wchar_t pAppName[] = L"Neobox";
          std::wstring wsThisPath = GetExeFullPath();
          std::wstring wsThatPath =
              RegReadString(HKEY_CURRENT_USER, pPath, pAppName);
          if (wsThatPath != wsThisPath) {
            if (checked) {
              RegWriteString(HKEY_CURRENT_USER, pPath, pAppName, wsThisPath);
            }
          } else {
            if (!checked) {
              RegRemoveValue(HKEY_CURRENT_USER, pPath, pAppName);
            }
          }
        },
        []() -> bool {
          wchar_t pPath[] =
              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
          return GetExeFullPath() ==
                 RegReadString(HKEY_CURRENT_USER, pPath, L"Neobox");
        }}},
      {u8"WallpaperAutoChange",
       {[this](bool checked) {
          m_Wallpaper->SetAutoChange(checked);
          VarBox::GetSettings(u8"Wallpaper")[u8"AutoChange"].second.setValue(
              checked);
          VarBox::WriteSettings();
        },
        std::bind(&Wallpaper::GetAutoChange, m_Wallpaper)}},
      {u8"WallpaperInitChange",
       {[this](bool checked) {
          m_Wallpaper->SetFirstChange(checked);
          VarBox::GetSettings(u8"Wallpaper")[u8"FirstChange"].second.setValue(
              checked);
          VarBox::WriteSettings();
        },
        std::bind(&Wallpaper::GetFirstCHange, m_Wallpaper)}}};

  m_FuncItemCheckMap = {
      {u8"WallpaperType",
       {[this](int type) {
          if (type == m_Wallpaper->GetImageType())
            return;
          m_Wallpaper->SetImageType(type);
          VarBox::GetSettings(u8"Wallpaper")[u8"ImageType"].second.setValue(
              type);
          VarBox::WriteSettings();
        },
        std::bind(&Wallpaper::GetImageType, m_Wallpaper)}}};
}

bool NeoMenu::ChooseFolder(QString title, QString& current) {
  current = QFileDialog::getExistingDirectory(this, title, current);
  return QDir().exists(current);
}

void NeoMenu::GetMenuContent(QMenu* parent, const YJson& data) {
  for (const auto& [i, j] : data.getObject()) {
    QAction* action = parent->addAction(QString::fromUtf8(i.data(), i.size()));
    const std::u8string type = j[u8"type"].second.getValueString();
    if (type == u8"Normal") {
      const auto& function =
          m_FuncNormalMap[j[u8"function"].second.getValueString()];
      connect(action, &QAction::triggered, this, function);
    } else if (type == u8"Group") {
      QMenu* menu = new QMenu(parent);
      action->setMenu(menu);
      GetMenuContent(menu, j[u8"children"].second);
    } else if (type == u8"Checkable") {
      const auto& [m, n] =
          m_FuncCheckMap[j[u8"function"].second.getValueString()];
      action->setCheckable(true);
      action->setChecked(n());
      connect(action, &QAction::triggered, this, m);
    } else if (type == u8"ExclusiveGroup") {
      const auto& function =
          m_FuncItemCheckMap[j[u8"function"].second.getValueString()].second;
      QMenu* menu = new QMenu(parent);
      action->setMenu(menu);
      const auto& children = j[u8"children"].second;
      GetMenuContent(menu, children);

      const auto& range = j[u8"range"].second.getArray();
      const int first = range.front().getValueInt();
      int last = range.back().getValueInt();
      if (last == -1)
        last = static_cast<int>(children.getObject().size());
      int index = 0, check = function();
      QActionGroup* group = new QActionGroup(menu);
      group->setExclusive(true);

      for (auto& k : menu->actions()) {
        if (index < first) {
          continue;
        } else if (index >= last) {
          break;
        } else {
          group->addAction(k);
          k->setChecked(index == check);
          ++index;
        }
      }
    } else if (type == u8"GroupItem") {
      const auto& function =
          m_FuncItemCheckMap[j[u8"function"].second.getValueString()].first;
      action->setCheckable(true);
      connect(action, &QAction::triggered, this,
              std::bind(function, j[u8"index"].second.getValueInt()));
    }
  }
}
