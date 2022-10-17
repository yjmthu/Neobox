#include <neomenu.h>
#include <screenfetch.h>
#include <shortcut.h>
#include <speedbox.h>
#include <systemapi.h>
#include <translatedlg.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>

#include <chrono>
#include <map>
#include <ranges>
#include <regex>
#include <vector>

#include <QActionGroup>
#include <QApplication>
#include <QColorDialog>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QSystemTrayIcon>

#ifdef _WIN32
#include <Windows.h>
#elif def __linux__
#else
#endif

namespace fs = std::filesystem;
using namespace std::chrono;
using namespace std::literals;

extern QString QImage2Text(const QImage& qImage);

NeoMenu::NeoMenu(QWidget* parent)
    : QMenu(parent),
      m_Shortcut(new Shortcut(this->parent())),
      m_TranslateDlg(new TranslateDlg(this)),
      m_Wallpaper(new Wallpaper(&VarBox::GetSettings(u8"Wallpaper"),
                                &VarBox::WriteSettings)) {
  setAttribute(Qt::WA_TranslucentBackground, true);
  setToolTipsVisible(true);
  InitFunctionMap();
  QFile fJson(QStringLiteral(":/jsons/menucontent.json"));
  if (!fJson.open(QIODevice::ReadOnly)) {
    qApp->quit();
    return;
  }
  QByteArray array = fJson.readAll();
  GetMenuContent(this, YJson(array.begin(), array.end()));
  fJson.close();

  // Maybe after the json file was download.
  LoadWallpaperExmenu();
}

NeoMenu::~NeoMenu() {
  delete m_Wallpaper;
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
      {u8"AppOpenWallpaperLocation",
        [this]() {
          std::wstring args = L"/select, " + m_Wallpaper->GetCurIamge().wstring();
          ShellExecuteW(nullptr, L"open", L"explorer", args.c_str(), NULL,
                        SW_SHOWNORMAL);
        }
      },
      {u8"AppOpenConfigDir",
       std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath()))},
      {u8"AppFormBackground",
       [this]() {
         auto& jsData = VarBox::GetSettings(u8"FormGlobal");
         int iCurType = jsData[u8"ColorEffect"].second.getValueInt();
         auto colList = jsData[u8"BackgroundColorRgba"].second.getArray() |
                        std::views::transform(
                            [](const YJson& i) { return i.getValueInt(); });
         auto colVector = std::vector<int>(colList.begin(), colList.end());
         const QColor col = QColorDialog::getColor(
             QColor(colVector[0], colVector[1], colVector[2], colVector[3]),
             this, "选择颜色", QColorDialog::ShowAlphaChannel);
         if (!col.isValid())
           return;
         jsData[u8"BackgroundColorRgba"].second.getArray() = {
             col.red(), col.green(), col.blue(), col.alpha()};
         VarBox::WriteSettings();

         HWND hWnd = reinterpret_cast<HWND>(
             qobject_cast<const QWidget*>(parent())->winId());
         SetWindowCompositionAttribute(
             hWnd, static_cast<ACCENT_STATE>(iCurType),
             qRgba(col.blue(), col.green(), col.red(), col.alpha()));
       }},
      {u8"ToolOcrGetScreen",
       [this]() {
         QImage image;
         ScreenFetch* box = new ScreenFetch(image);
         QEventLoop loop;
         connect(box, &ScreenFetch::destroyed, &loop, &QEventLoop::quit);
         box->showFullScreen();
         loop.exec();
         if (!box->HaveCatchImage())
           return;
         m_TranslateDlg->Show(
             qobject_cast<const QWidget*>(parent())->frameGeometry(),
             QImage2Text(image));
       }},
      {u8"ToolOcrShortcut",
        [this](){
          auto& settings = VarBox::GetSettings(u8"Tools");
          auto& shortcut = settings[u8"Ocr.Shortcut"].second.getValueString();
          auto& regist = settings[u8"Ocr.RegisterHotKey"].second;
          const QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          const QString qsNewShortcut = QInputDialog::getText(this, QStringLiteral("文字输入"), QStringLiteral("请输入热键组合"), QLineEdit::Normal, qsShortcut);
          if (qsNewShortcut.isEmpty() || qsNewShortcut == qsShortcut)
            return;
          const QByteArray&& array = qsNewShortcut.toUtf8();
          if (regist.isTrue()) {
            m_Shortcut->UnregistHotKey(qsShortcut);
            regist = false;
            m_CheckableActions[u8"ToolOcrRegistKey"]->setChecked(false);
          }
          shortcut.assign(array.begin(), array.end());
          VarBox::WriteSettings();
        }
      },
      {u8"ToolOcrDataPath",
       [this]() {
         std::u8string& u8Path =
             VarBox::GetSettings(u8"Tools")[u8"Ocr.TessdataDir"]
                 .second.getValueString();
         QByteArray folder =
             QFileDialog::getExistingDirectory(
                 this, "请选择Tessdata数据文件存放位置",
                 QString::fromUtf8(u8Path.data(), u8Path.size()))
                 .toUtf8();
         std::filesystem::path pNewPath = std::u8string(folder.begin(), folder.end());
         pNewPath.make_preferred();
         std::u8string u8NewPath = pNewPath.u8string();
         if (!u8NewPath.empty() && u8NewPath != u8Path) {
           u8Path.swap(u8NewPath);
           VarBox::WriteSettings();
         }
       }},
      {u8"ToolTransShowDlg",
       [this]() {
         if (m_TranslateDlg->isVisible()) {
           m_TranslateDlg->hide();
         } else {
           m_TranslateDlg->Show(
               qobject_cast<const QWidget*>(parent())->frameGeometry());
         }
       }},
      {u8"ToolTransShortcut",
        [this](){
          auto& settings = VarBox::GetSettings(u8"Tools");
          auto& shortcut = settings[u8"Translate.Shortcut"].second.getValueString();
          auto& regist = settings[u8"Translate.RegisterHotKey"].second;
          const QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          const QString qsNewShortcut = QInputDialog::getText(this, QStringLiteral("文字输入"), QStringLiteral("请输入热键组合"), QLineEdit::Normal, qsShortcut);
          if (qsNewShortcut.isEmpty() || qsNewShortcut == qsShortcut)
            return;
          const QByteArray&& array = qsNewShortcut.toUtf8();
          if (regist.isTrue()) {
            m_Shortcut->UnregistHotKey(qsShortcut);
            regist = false;
            m_CheckableActions[u8"ToolTransRegistKey"]->setChecked(false);
          }
          shortcut.assign(array.begin(), array.end());
          VarBox::WriteSettings();
        }
      },
      {u8"WallpaperPrev", std::bind(&Wallpaper::SetSlot, m_Wallpaper, -1)},
      {u8"WallpaperNext", std::bind(&Wallpaper::SetSlot, m_Wallpaper, 1)},
      {u8"WallpaperDislike", std::bind(&Wallpaper::SetSlot, m_Wallpaper, 0)},
      {u8"WallpaperUndoDislike",
       std::bind(&Wallpaper::UndoDelete, m_Wallpaper)},
      {u8"WallpaperCollect", std::bind(&Wallpaper::SetFavorite, m_Wallpaper)},
      {u8"WallpaperUndoCollect",
       std::bind(&Wallpaper::UnSetFavorite, m_Wallpaper)},
      {u8"WallpaperDir",
       [this]() {
         QString current =
             QString::fromStdWString(m_Wallpaper->GetImageDir().wstring());
         if (!ChooseFolder("选择壁纸文件夹", current))
           return;
         m_Wallpaper->SetCurDir(current.toStdWString());
       }},
      {u8"WallpaperTimeInterval",
       [this]() {
         int iNewTime =
             QInputDialog::getInt(this, "输入时间间隔", "时间间隔（分钟）：",
                                  m_Wallpaper->GetTimeInterval(), 5);
         if (iNewTime < 5)
           return;
         m_Wallpaper->SetTimeInterval(iNewTime);
       }},
      {u8"WallpaperClean", std::bind(&Wallpaper::ClearJunk, m_Wallpaper)},
      {u8"AppWbsite", std::bind(QDesktopServices::openUrl,
                                QUrl("https://www.github.com/yjmthu/Neobox"))}};

  m_FuncCheckMap = {
      {u8"SystemStopSleep",
       {[this](bool checked) {
          SetThreadExecutionState(checked ?
              (ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) :
              ES_CONTINUOUS
          );
        },
        []()->bool { return false; }
       }
      },
      {u8"AppAutoSatrt",
       {[this](bool checked) {
          wchar_t pPath[] =
              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
          wchar_t pAppName[] = L"Neobox";
          std::wstring wsThisPath = GetExeFullPath();
          std::wstring wsThatPath =
              RegReadString(HKEY_CURRENT_USER, pPath, pAppName);
          if (wsThatPath != wsThisPath) {
            if (checked && !RegWriteString(HKEY_CURRENT_USER, pPath, pAppName, wsThisPath)) {
              m_CheckableActions[u8"AppAutoSatrt"]->setChecked(false);
            }
          } else {
            if (!checked && !RegRemoveValue(HKEY_CURRENT_USER, pPath, pAppName)) {
              m_CheckableActions[u8"AppAutoSatrt"]->setChecked(true);
            }
          }
        },
        []() -> bool {
          wchar_t pPath[] =
              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
          return GetExeFullPath() ==
                 RegReadString(HKEY_CURRENT_USER, pPath, L"Neobox");
        }}},
      {u8"AppShowTrayIcon",
        {[this](bool checked) {
          ShowTrayIcon(checked);
          VarBox::GetSettings(u8"FormGlobal")[u8"ShowTrayIcon"].second = checked;
          VarBox::WriteSettings();
        },
        [this]() -> bool {
          const bool show = VarBox::GetSettings(u8"FormGlobal")[u8"ShowTrayIcon"].second.isTrue();
          if (show) {
            ShowTrayIcon(true);
          }
          return show;
        }}},
      {u8"OtherSetDesktopRightMenu",
       {[](bool checked) {
          constexpr auto prefix = L"Software\\Classes\\{}\\shell";
          const std::initializer_list<std::pair<std::wstring, wchar_t>> lst = {
              {L"*", L'1'},
              {L"Directory", L'V'},
              {L"Directory\\Background", L'V'}};
          constexpr auto command =
              L"mshta vbscript:clipboarddata.setdata(\"text\",\"%{}\")(close)";
          if (checked) {
            for (const auto& [param1, param2] : lst) {
              HKEY hKey = nullptr;
              std::wstring wsSubKey = std::format(prefix, param1) + L"\\Neobox";
              std::wstring cmdstr = std::format(command, param2);
              std::wstring wsIconFileName =
                  std::filesystem::absolute("icons/copy.ico").wstring();
              if (RegCreateKeyW(HKEY_CURRENT_USER, wsSubKey.c_str(), &hKey) !=
                  ERROR_SUCCESS) {
                return;
              }
              RegSetValueW(hKey, L"command", REG_SZ, cmdstr.c_str(), 0);
              RegSetValueW(hKey, nullptr, REG_SZ, L"复制路径", 0);
              RegSetValueExW(
                  hKey, L"Icon", 0, REG_SZ,
                  reinterpret_cast<const BYTE*>(wsIconFileName.data()),
                  (DWORD)wsIconFileName.size() * sizeof(wchar_t));
              RegCloseKey(hKey);
            }
          } else {
            for (const auto& [param1, param2] : lst) {
              HKEY hKey = nullptr;
              std::wstring wsSubKey = std::format(prefix, param1);
              if (RegOpenKeyExW(HKEY_CURRENT_USER, wsSubKey.c_str(), 0,
                                KEY_WRITE, &hKey) != ERROR_SUCCESS) {
                continue;
              }
              RegDeleteTreeW(hKey, L"Neobox");
              RegCloseKey(hKey);
            }
          }
        },
        []() -> bool {
          constexpr auto prefix = L"Software\\Classes\\{}\\shell";
          const auto lst = {L"*", L"Directory", L"Directory\\Background"};
          for (const auto& i : lst) {
            std::wstring wsSubKey = std::format(prefix, i);
            HKEY hKey = nullptr;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, wsSubKey.c_str(), 0, KEY_READ,
                              &hKey) != ERROR_SUCCESS) {
              QMessageBox::information(nullptr, "1",
                                       QString::fromStdWString(wsSubKey));
              return false;
            }
            RegCloseKey(hKey);
          }
          return true;
        }}},
      {u8"WallpaperAutoChange",
       {std::bind(&Wallpaper::SetAutoChange, m_Wallpaper,
                  std::placeholders::_1),
        std::bind(&Wallpaper::GetAutoChange, m_Wallpaper)}},
      {u8"WallpaperInitChange",
       {std::bind(&Wallpaper::SetFirstChange, m_Wallpaper,
                  std::placeholders::_1),
        std::bind(&Wallpaper::GetFirstChange, m_Wallpaper)}},
      {u8"ToolTransRegistKey",
       {[this](bool checked) {
          auto& settings = VarBox::GetSettings(u8"Tools");
          std::u8string& shortcut =
              settings[u8"Translate.Shortcut"].second.getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (checked) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolTransShowDlg"]);
          } else {
            m_Shortcut->UnregistHotKey(qsShortcut);
          }
          settings[u8"Translate.RegisterHotKey"].second = checked;
          VarBox::WriteSettings();
        },
        [this]() -> bool {
          auto& settings = VarBox::GetSettings(u8"Tools");
          bool regist = settings[u8"Translate.RegisterHotKey"].second.isTrue();
          std::u8string& shortcut =
              settings[u8"Translate.Shortcut"].second.getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (regist && !m_Shortcut->IsKeyRegisted(qsShortcut)) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolTransShowDlg"]);
          }
          return regist;
        }}},
      {u8"ToolOcrRegistKey",
       {[this](bool checked) {
          auto& settings = VarBox::GetSettings(u8"Tools");
          std::u8string& shortcut =
              settings[u8"Ocr.Shortcut"].second.getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (checked) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolOcrGetScreen"]);
          } else {
            m_Shortcut->UnregistHotKey(qsShortcut);
          }
          settings[u8"Ocr.RegisterHotKey"].second = checked;
          VarBox::WriteSettings();
        },
        [this]() -> bool {
          auto& settings = VarBox::GetSettings(u8"Tools");
          bool regist = settings[u8"Ocr.RegisterHotKey"].second.isTrue();
          std::u8string& shortcut =
              settings[u8"Ocr.Shortcut"].second.getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (regist && !m_Shortcut->IsKeyRegisted(qsShortcut)) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolOcrGetScreen"]);
          }
          return regist;
        }}}};

  m_FuncItemCheckMap = {
      {u8"WallpaperType",
       {[this](int type) {
          const int oldType = m_Wallpaper->GetImageType();
          if (m_Wallpaper->SetImageType(type)) {
            LoadWallpaperExmenu();
          } else {
            m_ExclusiveGroups[u8"WallpaperType"]->actions().at(oldType)->setChecked(true);
          }
        },
        std::bind(&Wallpaper::GetImageType, m_Wallpaper)}},
      {u8"AppFormEffect",
       {[this](int type) {
          // to do sth if failed.
          auto& jsData = VarBox::GetSettings(u8"FormGlobal");
          int iCurType = jsData[u8"ColorEffect"].second.getValueInt();
          if (iCurType != type) {
            jsData[u8"ColorEffect"].second = type;
            VarBox::WriteSettings();
          }
          auto colList = jsData[u8"BackgroundColorRgba"].second.getArray() |
                         std::views::transform(
                             [](const YJson& i) { return i.getValueInt(); });
          auto colVector = std::vector<int>(colList.begin(), colList.end());
          HWND hWnd = reinterpret_cast<HWND>(
              qobject_cast<const QWidget*>(parent())->winId());
          SetWindowCompositionAttribute(
              hWnd, static_cast<ACCENT_STATE>(type),
              qRgba(colVector[2], colVector[1], colVector[0], colVector[3]));
        },
        []() -> int {
          return VarBox::GetSettings(u8"FormGlobal")[u8"ColorEffect"]
              .second.getValueInt();
        }}}};
}

bool NeoMenu::ChooseFolder(QString title, QString& current) {
  current = QFileDialog::getExistingDirectory(this, title, current);
  return !current.isEmpty() && QDir().exists(current);
}

void NeoMenu::GetMenuContent(QMenu* parent, const YJson& data) {
  for (const auto& [i, j] : data.getObject()) {
    QAction* action = parent->addAction(QString::fromUtf8(i.data(), i.size()));
    const std::u8string type = j[u8"type"].second.getValueString();
    if (auto iter = j.find(u8"tip"); iter != j.endO()) {
      const std::u8string_view tip = iter->second.getValueString();
      action->setToolTip(QString::fromUtf8(tip.data(), tip.size()));
    }
    if (type == u8"Normal") {
      const auto& function =
          m_FuncNormalMap[j[u8"function"].second.getValueString()];
      connect(action, &QAction::triggered, this, function);
    } else if (type == u8"Group") {
      QMenu* menu = new QMenu(parent);
      menu->setToolTipsVisible(true);
      menu->setAttribute(Qt::WA_TranslucentBackground, true);
      action->setMenu(menu);
      GetMenuContent(menu, j[u8"children"].second);
    } else if (type == u8"Checkable") {
      const auto fnName = j[u8"function"].second.getValueString();
      const auto& [m, n] = m_FuncCheckMap[fnName];
      m_CheckableActions[fnName] = action;
      action->setCheckable(true);
      action->setChecked(n());
      connect(action, &QAction::triggered, this, m);
    } else if (type == u8"ExclusiveGroup") {
      const auto& function =
          m_FuncItemCheckMap[j[u8"function"].second.getValueString()].second;
      QMenu* menu = new QMenu(parent);
      menu->setToolTipsVisible(true);
      menu->setAttribute(Qt::WA_TranslucentBackground, true);
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
      m_ExclusiveGroups[j[u8"function"].second.getValueString()] = group;

      for (auto& k : menu->actions()) {
        if (index < first) {
          continue;
        } else if (index >= last) {
          break;
        }
        group->addAction(k);
        k->setChecked(index == check);
        ++index;
      }
    } else if (type == u8"GroupItem") {
      const auto& function =
          m_FuncItemCheckMap[j[u8"function"].second.getValueString()].first;
      action->setCheckable(true);
      connect(action, &QAction::triggered, this,
              std::bind(function, j[u8"index"].second.getValueInt()));
    } else if (type == u8"VarGroup") {
      QMenu* menu = new QMenu(parent);
      menu->setToolTipsVisible(true);
      menu->setAttribute(Qt::WA_TranslucentBackground, true);
      action->setMenu(menu);
      m_ExMenus[j[u8"name"].second.getValueString()] = menu;
    }
  }
}

void NeoMenu::LoadWallpaperExmenu() {
  QMenu* pMainMenu = m_ExMenus[u8"Wallpaper"];
  pMainMenu->clear();
  YJson* jsExInfo = m_Wallpaper->m_Wallpaper->GetJson();
  switch (m_Wallpaper->GetImageType()) {
    case WallBase::WALLHAVEN: {
      QActionGroup* group = new QActionGroup(pMainMenu);
      QAction* temp = nullptr;
      for (auto& [u8TypeName, data] :
           jsExInfo->find(u8"WallhavenApi")->second.getObject()) {
        QAction* const action = pMainMenu->addAction(
            QString::fromUtf8(u8TypeName.data(), u8TypeName.size()));
        action->setCheckable(true);
        action->setChecked(
            u8TypeName ==
            jsExInfo->find(u8"WallhavenCurrent")->second.getValueString());
        group->addAction(action);
        QMenu* pSonMenu = new QMenu(pMainMenu);
        pSonMenu->setAttribute(Qt::WA_TranslucentBackground, true);
        action->setMenu(pSonMenu);
        temp =
            pSonMenu->addAction(action->isChecked() ? "刷新此项" : "启用此项");
        connect(temp, &QAction::triggered, this, [this, action]() {
          action->setChecked(true);
          QByteArray&& array = action->text().toUtf8();
          auto js = m_Wallpaper->m_Wallpaper->GetJson();
          js->find(u8"WallhavenCurrent")
              ->second.setText(array.begin(), array.end());
          m_Wallpaper->m_Wallpaper->SetJson(true);
        });
        if (!action->isChecked()) {
          temp = pSonMenu->addAction(QStringLiteral("删除此项"));
          connect(temp, &QAction::triggered, this, [this, action](){
            QByteArray&& array = action->text().toUtf8();
            m_Wallpaper->m_Wallpaper->GetJson()->find(u8"WallhavenApi")->second.remove(std::u8string_view(
                  reinterpret_cast<const char8_t*>(array.data()),
                  array.size()
            ));
            m_Wallpaper->m_Wallpaper->SetJson(false);
            LoadWallpaperExmenu();
          });
        }
        temp = pSonMenu->addAction("参数设置");
        connect(temp, &QAction::triggered, this,
                std::bind(&NeoMenu::WallhavenParams, this, action));
        temp = pSonMenu->addAction("存储路径");
        connect(temp, &QAction::triggered, this, [this, action]() {
          QString sCurDir =
              QString::fromStdWString(m_Wallpaper->GetImageDir().wstring());
          QByteArray&& array = action->text().toUtf8();
          std::u8string key(array.begin(), array.end());
          if (ChooseFolder("请选择壁纸存放路径", sCurDir)) {
            std::filesystem::path path = sCurDir.toStdWString();
            path.make_preferred();

            m_Wallpaper->m_Wallpaper->GetJson()
                ->find(key)
                ->second[u8"Directory"]
                .second = path.u8string();
            m_Wallpaper->m_Wallpaper->SetJson(false);
          }
        });
      }
      pMainMenu->addSeparator();
      temp = pMainMenu->addAction("添加更多");
      connect(temp, &QAction::triggered, this,
              std::bind(&NeoMenu::WallhavenParams, this, nullptr));
      temp = pMainMenu->addAction("关于壁纸");
      connect(temp, &QAction::triggered, this, [this]() {
        auto fileName = m_Wallpaper->GetCurIamge().stem().string();
        std::regex pattern("^wallhaven-[a-z0-9]{6}$", std::regex::icase);
        if (!std::regex_match(fileName, pattern))
          return;
        std::string url = "https://wallhaven.cc/w/" + fileName.substr(10);
        QDesktopServices::openUrl(QString::fromStdString(url));
      });
      temp = pMainMenu->addAction("相似壁纸");
      connect(temp, &QAction::triggered, this, [this]() {
        auto fileName = m_Wallpaper->GetCurIamge().stem().string();
        std::regex pattern("^wallhaven-[a-z0-9]{6}$", std::regex::icase);
        if (!std::regex_match(fileName, pattern))
          return;
        std::string url =
            "https://wallhaven.cc/search?q=like:" + fileName.substr(10);
        QDesktopServices::openUrl(QString::fromStdString(url));
      });
      break;
    }
    case WallBase::BINGAPI: {
      QAction* temp = nullptr;
      temp = pMainMenu->addAction("名称格式");
      connect(temp, &QAction::triggered, this, [this, jsExInfo]() {
        auto& u8ImgFmt = jsExInfo->find(u8"imgfmt")->second.getValueString();
        QString qImgFmt = QString::fromUtf8(u8ImgFmt.data(), u8ImgFmt.size());
        QByteArray&& qImgFmtNewByte =
            QInputDialog::getText(this, "图片名称", "输入名称格式",
                                  QLineEdit::Normal, qImgFmt)
                .toUtf8();
        std::u8string u8ImgFmtNew(qImgFmtNewByte.begin(), qImgFmtNewByte.end());
        if (u8ImgFmtNew.empty() || u8ImgFmtNew == u8ImgFmt)
          return;
        u8ImgFmt.swap(u8ImgFmtNew);
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction("地理位置");
      connect(temp, &QAction::triggered, this, [this, jsExInfo]() {
        auto& u8Mkt = jsExInfo->find(u8"mkt")->second.getValueString();
        QString qMkt = QString::fromUtf8(u8Mkt.data(), u8Mkt.size());
        QByteArray&& qMktNewByte =
            QInputDialog::getText(this, "图片地区", "输入图片所在地区",
                                  QLineEdit::Normal, qMkt)
                .toUtf8();
        std::u8string u8MktNew(qMktNewByte.begin(), qMktNewByte.end());
        if (u8MktNew.empty() || u8MktNew == u8Mkt)
          return;
        u8Mkt.swap(u8MktNew);
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction("自动下载");
      temp->setCheckable(true);
      temp->setChecked(jsExInfo->find(u8"auto-download")->second.isTrue());
      connect(temp, &QAction::triggered, this, [this, jsExInfo](bool checked) {
        jsExInfo->find(u8"auto-download")->second = checked;
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction("关于壁纸");
      connect(temp, &QAction::triggered, this, [jsExInfo]() {
        size_t index = jsExInfo->find(u8"index")->second.getValueInt();
        const auto time = current_zone()->to_local(system_clock::now() - 24h);
        std::string curDate =
            std::format("&filters=HpDate:\"{0:%Y%m%d}_1600\"", time);
        std::u8string link = jsExInfo->find(u8"images")
                                 ->second[index][u8"copyrightlink"]
                                 .second.getValueString();
        link.append(curDate.cbegin(), curDate.cend());
        QDesktopServices::openUrl(QString::fromUtf8(link.data(), link.size()));
      });
      break;
    }
    case WallBase::NATIVE: {
      QAction* temp = pMainMenu->addAction(QStringLiteral("递归遍历"));
      temp->setCheckable(true);
      temp->setChecked(jsExInfo->find(u8"recursion")->second.isTrue());
      connect(temp, &QAction::triggered, this, [this, jsExInfo](bool checked) {
        jsExInfo->find(u8"recursion")->second = checked;
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction(QStringLiteral("随机抽选"));
      temp->setCheckable(true);
      temp->setChecked(jsExInfo->find(u8"random")->second.isTrue());
      connect(temp, &QAction::triggered, this, [this, jsExInfo](bool checked) {
        jsExInfo->find(u8"random")->second = checked;
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      break;
    }
    case WallBase::DIRECTAPI: {
      std::u8string_view key =
          jsExInfo->find(u8"ApiUrl")->second.getValueString();
      QAction* pSuperTemp, *temp = nullptr;
      QActionGroup* pGroup = new QActionGroup(pMainMenu);
      for (const auto& [_name, data] :
           jsExInfo->find(u8"ApiData")->second.getObject()) {
        std::u8string name = _name;
        pSuperTemp =
            pMainMenu->addAction(QString::fromUtf8(name.data(), name.size()));
        pSuperTemp->setCheckable(true);
        pSuperTemp->setChecked(name == key);
        pGroup->addAction(pSuperTemp);

        QMenu* tempMenu = new QMenu(pMainMenu);
        pSuperTemp->setMenu(tempMenu);
        connect(tempMenu->addAction(QStringLiteral("修改名称")),
            &QAction::triggered, this, [this, pSuperTemp, name, jsExInfo](){
              QString qNewKeyName = QString::fromUtf8(name.data(), name.size());
              qNewKeyName = QInputDialog::getText(this,
                  QStringLiteral("文字输入"),
                  QStringLiteral("请输入新名称"),
                  QLineEdit::Normal,
                  qNewKeyName
              );
              if (qNewKeyName.isEmpty()) return;
              QByteArray array = qNewKeyName.toUtf8();
              std::u8string_view viewNewName(
                  reinterpret_cast<const char8_t*>(array.data()),
                  array.size()
              );
              auto& jsApiData = jsExInfo->find(u8"ApiData")->second;
              auto iter = jsApiData.find(viewNewName);
              if (iter != jsApiData.endO())
                return;
              if (pSuperTemp->isChecked()) {
                jsExInfo->find(u8"ApiUrl")->second = viewNewName;
              }
              jsApiData[name].first = viewNewName;
              m_Wallpaper->m_Wallpaper->SetJson(false);
              pSuperTemp->setText(qNewKeyName);
            });
        if (!pSuperTemp->isChecked()) {
          connect(tempMenu->addAction(QStringLiteral("启用此项")),
              &QAction::triggered, this, [=]() {
                jsExInfo->find(u8"ApiUrl")->second = name;
                m_Wallpaper->m_Wallpaper->SetJson(false);
                LoadWallpaperExmenu();
              });
          connect(tempMenu->addAction(QStringLiteral("删除此项")),
              &QAction::triggered, this, [=]() {
                pGroup->removeAction(pSuperTemp);
                delete pSuperTemp;
                jsExInfo->find(u8"ApiData")->second.remove(name);
                m_Wallpaper->m_Wallpaper->SetJson(false);
              });
        }
        temp = tempMenu->addAction(QStringLiteral("路径编辑"));
        tempMenu->addSeparator();
        QActionGroup* pSonGroup = new QActionGroup(pMainMenu);

        connect(temp, &QAction::triggered, this, [this, name, jsExInfo](){
            bool changed = GetListWidget(QStringLiteral("输入文字"), QStringLiteral("请输入参数"),
                jsExInfo->find(u8"ApiData")->second[name].second[u8"Paths"].second);
            if (changed) {
              LoadWallpaperExmenu();
            }
        });

        for (int32_t index = 0, cIndex = data[u8"CurPath"].second.getValueInt();
             auto& path : data[u8"Paths"].second.getArray()) {
          std::u8string_view path_view = path.getValueString();
          temp = tempMenu->addAction(
              QString::fromUtf8(path_view.data(), path_view.size()));
          temp->setCheckable(true);
          pSonGroup->addAction(temp);
          connect(temp, &QAction::triggered, this, [=](bool checked) {
            jsExInfo->find(u8"ApiData")
                ->second[name]
                .second[u8"CurPath"]
                .second = index;
            m_Wallpaper->m_Wallpaper->SetJson(false);
          });
          temp->setChecked(cIndex == index++);
        }
      }
      temp = pMainMenu->addAction(QStringLiteral("添加更多"));
      connect(temp, &QAction::triggered, this, [this, jsExInfo](){
        while (true) {
          const QString qKeyName = QInputDialog::getText(this,
              QStringLiteral("请输入文字"), 
              QStringLiteral("请输入要添加的Api的名称"),
              QLineEdit::Normal, QStringLiteral("New Api"));
          if (qKeyName.isEmpty()) return;
          QByteArray qbBuffer = qKeyName.toUtf8();
          std::u8string_view viewKeyName(
            reinterpret_cast<const char8_t*>(qbBuffer.data()),
            qbBuffer.size()
          );
          auto& obj = jsExInfo->find(u8"ApiData")->second[viewKeyName].second;
          if (!obj.isNull()) {
            continue;
          }

          QString qDomain = QInputDialog::getText(this,
              QStringLiteral("请输入文字"), 
              QStringLiteral("请输入要添加的Api的域名"),
              QLineEdit::Normal, QStringLiteral("https://w.wallhaven.cc"));
          if (qDomain.isEmpty()) qDomain = QStringLiteral("https://w.wallhaven.cc");
          qbBuffer = qDomain.toUtf8();
          std::u8string u8Domain(qbBuffer.begin(), qbBuffer.end());
          QString qsFolder;
          while (!ChooseFolder(QStringLiteral("选择壁纸存放文件夹"), qsFolder))
            continue;
          fs::path folder = qsFolder.toStdU16String();
          folder.make_preferred();
          obj = YJson(YJson::O {
            {u8"Url"sv, u8Domain},
            {u8"CurPath"sv, 0},
            {u8"Paths"sv, YJson::A { u8"/full/6o/wallhaven-6oxgp6.jpg"s }},
            {u8"Directory"sv, folder.u8string()},
            {u8"ImageNameFormat"sv, u8"{0:%Y-%m-%d} {0:%H%M%S}.jpg"s}
          });
          GetListWidget(QStringLiteral("参数输入"), QStringLiteral("请输入Api路径"), obj[u8"Paths"].second);
          LoadWallpaperExmenu();
          return;
        }
      });
      break;
    }
    case WallBase::SCRIPTOUTPUT: {
      QAction* temp = pMainMenu->addAction(QStringLiteral("程序路径"));
      connect(temp, &QAction::triggered, this, [this, jsExInfo]() {
        QString fileName = QFileDialog::getOpenFileName(
            this, "请选择可执行文件", QString(), "*.*");
        if (fileName.isEmpty() || !QFile::exists(fileName))
          return;
        QByteArray array = fileName.toUtf8();
        fs::path exe = std::u8string_view(reinterpret_cast<const char8_t*>(array.data()), array.size());
        exe.make_preferred();
        jsExInfo->find(u8"executeable")->second = exe.u8string();
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction(QStringLiteral("参数列表"));
      connect(temp, &QAction::triggered, this, std::bind(&NeoMenu::GetListWidget, this, QStringLiteral("输入文字"), QStringLiteral("请输入参数"), std::ref(jsExInfo->find(u8"arglist")->second)));
      break;
    }
    default:
      break;
  }
}

void NeoMenu::WallhavenParams(QAction*const action) {
  QDialog* pTableDlg = new QDialog(this);
  pTableDlg->setWindowTitle(QStringLiteral("参数设置"));
  pTableDlg->setAttribute(Qt::WA_DeleteOnClose, true);
  QTableWidget* pTable = nullptr;
  QVBoxLayout* pVLayout = new QVBoxLayout(pTableDlg);
  QHBoxLayout* pHLayout = new QHBoxLayout;
  std::array arButtons = {
      new QPushButton("添加", pTableDlg), new QPushButton("删除", pTableDlg),
      new QPushButton("确认", pTableDlg), new QPushButton("取消", pTableDlg)};
  std::for_each(arButtons.begin(), arButtons.end(),
                std::bind(&QHBoxLayout::addWidget, pHLayout,
                          std::placeholders::_1, 0, Qt::Alignment()));
  pVLayout->addLayout(pHLayout);
  std::u8string u8TypeName;
  if (action == nullptr) {
    QByteArray qArrayKeyName =
        QInputDialog::getText(this, "请输入类型名称", "新名称",
                              QLineEdit::Normal, "新壁纸")
            .toUtf8();
    if (qArrayKeyName.isEmpty()) {
      return;
    }
    u8TypeName.assign(qArrayKeyName.begin(), qArrayKeyName.end());
    pTable = new QTableWidget(0, 2, pTableDlg);
  } else {
    QByteArray qArrayKeyName = action->text().toUtf8();
    u8TypeName.assign(qArrayKeyName.begin(), qArrayKeyName.end());
    auto& params = m_Wallpaper->m_Wallpaper->GetJson()
                       ->find(u8"WallhavenApi")
                       ->second[u8TypeName]
                       .second[u8"Parameter"]
                       .second.getObject();
    pTable = new QTableWidget(static_cast<int>(params.size()), 2, pTableDlg);
    for (int i = 0; const auto& [key, value] : params) {
      std::u8string_view value_view = value.getValueString();
      pTable->setItem(
          i, 0,
          new QTableWidgetItem(QString::fromUtf8(key.data(), key.size())));
      pTable->setItem(i, 1,
                      new QTableWidgetItem(QString::fromUtf8(
                          value_view.data(), value_view.size())));
      ++i;
    }
  }
  pTable->setHorizontalHeaderLabels(
      {QStringLiteral("key"), QStringLiteral("value")});
  pVLayout->insertWidget(0, pTable);

  connect(arButtons[0], &QPushButton::clicked, this,
      [pTable]() {
          if (pTable->currentRow() == -1) {
            pTable->insertRow(pTable->rowCount());
          } else {
            pTable->insertRow(pTable->currentRow());
          }
      });
  connect(arButtons[1], &QPushButton::clicked, this,
          [pTable]() { pTable->removeRow(pTable->currentRow()); });
  connect(arButtons[2], &QPushButton::clicked, this, [=]() {
    YJson jsData = YJson::Object;
    for (int i = 0; i < pTable->rowCount(); ++i) {
      auto ptr = pTable->item(i, 0);
      if (ptr == nullptr) continue;
      auto qsKeyArray = ptr->text().toUtf8();
      ptr = pTable->item(i, 1);
      if (ptr == nullptr) continue;
      auto qsValueArray = ptr->text().toUtf8();
      if (qsKeyArray.isEmpty() || qsValueArray.isEmpty())
        continue;
      jsData.append(std::u8string(qsValueArray.begin(), qsValueArray.end()),
                    std::u8string(qsKeyArray.begin(), qsKeyArray.end()));
    }
    auto& jsApis =
        m_Wallpaper->m_Wallpaper->GetJson()->find(u8"WallhavenApi")->second;
    auto iter = jsApis.find(u8TypeName);
    if (iter == jsApis.endO()) {
      QString folder;
      if (!ChooseFolder("选择壁纸文件夹", folder)) {
        pTableDlg->close();
        return;
      }
      QByteArray arFolder = folder.toUtf8();
      jsApis.append(YJson::O{{u8"Parameter", jsData},
                             {u8"Directory",
                              std::u8string(arFolder.begin(), arFolder.end())}},
                    u8TypeName);
      m_Wallpaper->m_Wallpaper->SetJson(false);
    } else {
      YJson::swap(jsData, iter->second[u8"Parameter"].second);
      m_Wallpaper->m_Wallpaper->SetJson(u8TypeName ==
                                        m_Wallpaper->m_Wallpaper->GetJson()
                                            ->find(u8"WallhavenCurrent")
                                            ->second.getValueString());
    }
    pTableDlg->close();
  });
  connect(arButtons[3], &QPushButton::clicked, pTableDlg, &QDialog::close);
  pTableDlg->exec();
  if (action == nullptr) {
    LoadWallpaperExmenu();
  }
}


bool NeoMenu::GetListWidget(QString title, QString label, YJson& data)
{
  bool bDataChanged = false;
  QDialog argDlg(this);
  argDlg.setWindowTitle(QStringLiteral("参数编辑器"));
  QVBoxLayout* pVout = new QVBoxLayout(&argDlg);
  QHBoxLayout* pHout = new QHBoxLayout;
  QListWidget* pLstWgt = new QListWidget(&argDlg);
  pVout->addWidget(pLstWgt);
  std::array arButtons = {
      new QPushButton(QStringLiteral("添加"), &argDlg),
      new QPushButton(QStringLiteral("删除"), &argDlg),
      new QPushButton(QStringLiteral("取消"), &argDlg),
      new QPushButton(QStringLiteral("确认"), &argDlg),
  };

  std::for_each(arButtons.begin(), arButtons.end(),
                std::bind(&QHBoxLayout::addWidget, pHout,
                          std::placeholders::_1, 0, Qt::Alignment()));

  auto argview = data.getArray() | std::views::transform([](const YJson& item) {
     std::u8string_view str = item.getValueString();
     return QString::fromUtf8(str.data(), str.size());
  });

  pLstWgt->addItems(QStringList(argview.begin(), argview.end()));
  pVout->addLayout(pHout);

  connect(arButtons[0], &QPushButton::clicked, pLstWgt, [title, label, &argDlg, pLstWgt]() {
    QListWidgetItem *item = new QListWidgetItem("New Item");
    pLstWgt->insertItem(pLstWgt->currentRow(), item);
    const QString str = QInputDialog::getText(
        &argDlg, 
        title, 
        label, 
        QLineEdit::Normal,
        item->text());
    if (!str.isEmpty() && str != item->text())
      item->setText(str);
  });
  connect(arButtons[1], &QPushButton::clicked, pLstWgt, [pLstWgt]() {
    for (auto item: pLstWgt->selectedItems()) {
      delete item;
    }
  });
  connect(arButtons[2], &QPushButton::clicked, &argDlg,
          [&argDlg]() { argDlg.close(); });
  connect(arButtons[3], &QPushButton::clicked, &argDlg,
          [&argDlg, this, pLstWgt, &data, &bDataChanged]() {
            std::list<YJson> args;
            for (int i = 0; i < pLstWgt->count(); ++i) {
              auto ptr = pLstWgt->item(i);
              if (ptr == nullptr) continue;
              QByteArray array = ptr->text().toUtf8();
              if (array.isEmpty()) continue;
              args.emplace_back(std::u8string(array.begin(), array.end()));
            }
            data.getArray() = std::move(args);
            m_Wallpaper->m_Wallpaper->SetJson(true);
            bDataChanged = true;
            argDlg.close();
          });
  connect(pLstWgt, &QListWidget::itemDoubleClicked, &argDlg, [title, label, &argDlg](QListWidgetItem* item){
    const QString str = QInputDialog::getText(
        &argDlg, 
        title,
        label,
        QLineEdit::Normal,
        item->text());
    if (!str.isEmpty() && str != item->text())
      item->setText(str);
  });
  argDlg.exec();
  return bDataChanged;
}

void NeoMenu::ShowTrayIcon(bool show)
{
  static QSystemTrayIcon* pSystemTray = nullptr;
  
  if (show) {
    pSystemTray = new QSystemTrayIcon(this);
    if (VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"].second.isFalse())
      VarBox::GetSpeedBox()->hide();
    pSystemTray->setIcon(QIcon(QStringLiteral(":/icons/speedbox.ico")));
    pSystemTray->setContextMenu(this);
    pSystemTray->setToolTip("Neobox");
    connect(pSystemTray, &QSystemTrayIcon::activated,
      [](QSystemTrayIcon::ActivationReason reason){
        switch (reason)
        {
          case QSystemTrayIcon::DoubleClick:
            if (VarBox::GetSpeedBox()->isVisible()) {
              VarBox::GetSpeedBox()->hide();
              VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"].second = false;
            } else {
              VarBox::GetSpeedBox()->show();
              VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"].second = true;
            }
            VarBox::WriteSettings();
          case QSystemTrayIcon::Trigger:
          default:
            break;
        }
      });
    pSystemTray->show();
  } else {
    pSystemTray->hide();
    pSystemTray->deleteLater();
    pSystemTray = nullptr;
  }
}
