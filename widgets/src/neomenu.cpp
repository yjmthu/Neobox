#include <neomenu.h>
#include <screenfetch.h>
#include <shortcut.h>
#include <speedbox.h>
#include <systemapi.h>
#include <translatedlg.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>
#include <appcode.hpp>
#include <versiondlg.h>

#include <chrono>
#include <map>
#include <ranges>
#include <regex>
#include <vector>

#include <QProcess>
#include <QActionGroup>
#include <QSharedmemory>
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
#include <QClipboard>
#include <QMimeData>

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
  auto const menuContent = VarBox::GetInstance()->LoadJsons();
  GetMenuContent(this, *menuContent);

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
      {u8"AppQuit", QApplication::quit},
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
      {u8"AppMoveToLeftTop",
        std::bind(&SpeedBox::InitMove, VarBox::GetSpeedBox())
      },
      {u8"AppOpenConfigDir",
       std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath()))},
      {u8"AppFormBackground",
       [this]() {
         auto& jsData = VarBox::GetSettings(u8"FormGlobal");
         int iCurType = jsData[u8"ColorEffect"].getValueInt();
         auto colList = jsData[u8"BackgroundColorRgba"].getArray() |
                        std::views::transform(
                            [](const YJson& i) { return i.getValueInt(); });
         auto colVector = std::vector<int>(colList.begin(), colList.end());
         const QColor col = QColorDialog::getColor(
             QColor(colVector[0], colVector[1], colVector[2], colVector[3]),
             this, "选择颜色", QColorDialog::ShowAlphaChannel);
         if (!col.isValid()) {
           VarBox::ShowMsg("设置颜色失败！");
           return;
         }
         jsData[u8"BackgroundColorRgba"].getArray() = {
             col.red(), col.green(), col.blue(), col.alpha()};
         VarBox::WriteSettings();

         HWND hWnd = reinterpret_cast<HWND>(
             qobject_cast<const QWidget*>(parent())->winId());
         SetWindowCompositionAttribute(
             hWnd, static_cast<ACCENT_STATE>(iCurType),
             qRgba(col.blue(), col.green(), col.red(), col.alpha()));
         VarBox::ShowMsg("设置颜色成功！");
       }},
      {u8"AppAddSkin",
        [this]() {
          const QString qSkinName = QInputDialog::getText(this, "输入", "请输入壁纸名字：");
          if (qSkinName.isEmpty() || qSkinName.isNull()) {
            VarBox::ShowMsg("添加皮肤失败！");
            return;
          }
          
          auto menu = m_ExMenus[u8"AppSelectSkin"];
          auto& object = VarBox::GetSettings(u8"FormGlobal")[u8"UserSkins"];
          const QByteArray buffer1 = qSkinName.toUtf8();
          const std::u8string u8SkinName(buffer1.begin(), buffer1.end());
          for (auto qobj: menu->children()) {
            if (qobject_cast<QAction*>(qobj)->text() == qSkinName) {
              VarBox::ShowMsg("请勿输入已经存在的名称！");
              return;
            }
          }

          const QString qFilePath = QFileDialog::getOpenFileName(this, "选择文件", ".", "(*.ui)");
          if (qFilePath.isEmpty() || !QFile::exists(qFilePath)) {
            VarBox::ShowMsg("添加皮肤失败！");
            return;
          }
          const QByteArray buffer2 = qFilePath.toUtf8();
          const fs::path path = std::u8string(buffer2.begin(), buffer2.end());
          const std::u8string u8FilePath = u8"styles/" + path.filename().u8string();
          
          
          object.append(u8FilePath, u8SkinName);
          VarBox::WriteSettings();

          if (object.sizeO() == 1)
            menu->addSeparator();
          auto action = menu->addAction(qSkinName);
          action->setToolTip(QString::fromUtf8(u8FilePath.data(), u8FilePath.size()));
          action->setCheckable(true);
          connect(action, &QAction::triggered, this,
              std::bind(m_FuncStingMap[u8"AppSelectSkin"], u8SkinName));
          
          menu = m_ExMenus[u8"AppRemoveSkin"];
          action = menu->addAction(qSkinName);
          connect(action, &QAction::triggered, this,
              std::bind(m_FuncStingMap[u8"AppRemoveSkin"], u8SkinName));
          VarBox::ShowMsg("添加皮肤成功！");
        }
      },
      {u8"ToolOcrGetScreen",
       [this]() {
         static bool busy = false;
         if (busy) return;
         busy = true;
         QImage image;
         ScreenFetch* box = new ScreenFetch(image);
         QEventLoop loop;
         connect(box, &ScreenFetch::destroyed, &loop, &QEventLoop::quit);
         box->showFullScreen();
         loop.exec();
         busy = false;
         if (!box->HaveCatchImage())
           return;
         m_TranslateDlg->Show(
             qobject_cast<const QWidget*>(parent())->frameGeometry(),
             QImage2Text(image));
       }},
      {u8"ToolOcrShortcut",
        [this](){
          auto& settings = VarBox::GetSettings(u8"Tools");
          auto& shortcut = settings[u8"Ocr.Shortcut"].getValueString();
          auto& regist = settings[u8"Ocr.RegisterHotKey"];
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
                 .getValueString();
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
           VarBox::ShowMsg("设置数据文件失成功！");
         } else {
           VarBox::ShowMsg("设置数据文件失败！");
         }
       }},
      {u8"ToolTransShowDlg",
       [this]() {
         if (m_TranslateDlg->isVisible()) {
           m_TranslateDlg->hide();
         } else {
           const auto set = VarBox::GetSettings(u8"Tools")[u8"Translate.ReadClipboard"];
           if (set.isTrue()) {
             const QClipboard *clipbord = QGuiApplication::clipboard();
             const QMimeData *mimeData = clipbord->mimeData();
             if (mimeData->hasText()) {
               m_TranslateDlg->Show(
                 qobject_cast<const QWidget*>(parent())->frameGeometry(),
                 mimeData->text());
               return;
             }
           }
           m_TranslateDlg->Show(
             qobject_cast<const QWidget*>(parent())->frameGeometry());
         }
       }},
      {u8"ToolTransShortcut",
        [this](){
          auto& settings = VarBox::GetSettings(u8"Tools");
          auto& shortcut = settings[u8"Translate.Shortcut"].getValueString();
          auto& regist = settings[u8"Translate.RegisterHotKey"];
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
          VarBox::ShowMsg("设置翻译快捷键成功！");
        }
      },
      {u8"ToolColorPick", std::bind(&QColorDialog::getColor, Qt::white, this, QStringLiteral("颜色拾取器"), QColorDialog::ColorDialogOptions())
      },
      {u8"WallpaperPrev", std::bind(&Wallpaper::SetSlot, m_Wallpaper, -1)},
      {u8"WallpaperNext", std::bind(&Wallpaper::SetSlot, m_Wallpaper, 1)},
      {u8"WallpaperDislike", std::bind(&Wallpaper::SetSlot, m_Wallpaper, 0)},
      {u8"WallpaperUndoDislike",
       std::bind(&Wallpaper::UndoDelete, m_Wallpaper)},
      {u8"WallpaperCollect", [this](){
          m_Wallpaper->Wallpaper::SetFavorite();
          VarBox::ShowMsg("收藏壁纸成功！");
        }},
      {u8"WallpaperUndoCollect", [this](){
          m_Wallpaper->UnSetFavorite();
          VarBox::ShowMsg("撤销收藏壁纸成功！");
        }},
      {u8"WallpaperDir",
       [this]() {
          QString current =
              QString::fromStdWString(m_Wallpaper->GetImageDir().wstring());
          if (!ChooseFolder("选择壁纸文件夹", current)) {
            VarBox::ShowMsg("取消设置成功！");
            return;
          }
          m_Wallpaper->SetCurDir(current.toStdWString());
          VarBox::ShowMsg("设置壁纸存放位置成功！");
        }},
      {u8"WallpaperTimeInterval",
       [this]() {
         int iNewTime =
             QInputDialog::getInt(this, "输入时间间隔", "时间间隔（分钟）：",
                                  m_Wallpaper->GetTimeInterval(), 5);
         if (iNewTime < 5) {
           VarBox::ShowMsg("设置时间间隔失败！");
           return;
         }
         m_Wallpaper->SetTimeInterval(iNewTime);
         VarBox::ShowMsg("设置时间间隔成功！");
       }},
      {u8"WallpaperClean", [this](){
          m_Wallpaper->ClearJunk();
          VarBox::ShowMsg("清除壁纸垃圾成功！");
       }},
      {u8"AppWbsite", std::bind(QDesktopServices::openUrl,
                                QUrl("https://www.github.com/yjmthu/Neobox"))},
      {u8"AppInfo", [](){
          // loop can make the shortcut key still work.
          QEventLoop loop;
          VersionDlg dlg;
          dlg.show();
          connect(&dlg, &QDialog::finished, &loop, &QEventLoop::quit);
          loop.exec();
      }},
    };

  m_FuncCheckMap = {
      {u8"SystemStopSleep",
       {[this](bool checked) {
          SetThreadExecutionState(checked ?
              (ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) :
              ES_CONTINUOUS
          );
          VarBox::GetSettings(u8"Tools")[u8"App.StopSleep"] = checked;
          VarBox::WriteSettings();
        },
        []()->bool {
          bool enable = VarBox::GetSettings(u8"Tools")[u8"App.StopSleep"].isTrue();
          if (enable) {
            SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
          }
          return enable;
        }
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
              VarBox::ShowMsg("设置自动启动失败！");
            } else {
              VarBox::ShowMsg("设置成功！");
            }
          } else {
            if (!checked && !RegRemoveValue(HKEY_CURRENT_USER, pPath, pAppName)) {
              m_CheckableActions[u8"AppAutoSatrt"]->setChecked(true);
              VarBox::ShowMsg("取消自动启动失败！");
            } else {
              VarBox::ShowMsg("设置成功！");
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
          VarBox::GetSettings(u8"FormGlobal")[u8"ShowTrayIcon"] = checked;
          VarBox::WriteSettings();
          VarBox::ShowMsg(checked? "显示托盘图标成功！": "隐藏托盘图标成功！");
        },
        [this]() -> bool {
          const bool show = VarBox::GetSettings(u8"FormGlobal")[u8"ShowTrayIcon"].isTrue();
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
              settings[u8"Translate.Shortcut"].getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (checked) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolTransShowDlg"]);
          } else {
            m_Shortcut->UnregistHotKey(qsShortcut);
          }
          settings[u8"Translate.RegisterHotKey"] = checked;
          VarBox::WriteSettings();
        },
        [this]() -> bool {
          auto& settings = VarBox::GetSettings(u8"Tools");
          bool regist = settings[u8"Translate.RegisterHotKey"].isTrue();
          std::u8string& shortcut =
              settings[u8"Translate.Shortcut"].getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (regist && !m_Shortcut->IsKeyRegisted(qsShortcut)) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolTransShowDlg"]);
          }
          return regist;
        }}},
      {u8"ToolTransAutoTranslate",
        {[this](bool checked) {
          VarBox::GetSettings(u8"Tools")[u8"Translate.AutoTranslate"] = checked;
          VarBox::WriteSettings();
        },
        []()->bool { return VarBox::GetSettings(u8"Tools")[u8"Translate.AutoTranslate"].isTrue(); }
        }},
      {u8"ToolTransReadClipboard",
        {[this](bool checked) {
          VarBox::GetSettings(u8"Tools")[u8"Translate.ReadClipboard"] = checked;
          VarBox::WriteSettings();
        },
        []()->bool{
          return VarBox::GetSettings(u8"Tools")[u8"Translate.ReadClipboard"].isTrue();
        }}},
      {u8"ToolOcrRegistKey",
       {[this](bool checked) {
          auto& settings = VarBox::GetSettings(u8"Tools");
          std::u8string& shortcut =
              settings[u8"Ocr.Shortcut"].getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (checked) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolOcrGetScreen"]);
          } else {
            m_Shortcut->UnregistHotKey(qsShortcut);
          }
          settings[u8"Ocr.RegisterHotKey"] = checked;
          VarBox::WriteSettings();
        },
        [this]() -> bool {
          auto& settings = VarBox::GetSettings(u8"Tools");
          bool regist = settings[u8"Ocr.RegisterHotKey"].isTrue();
          std::u8string& shortcut =
              settings[u8"Ocr.Shortcut"].getValueString();
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
          int iCurType = jsData[u8"ColorEffect"].getValueInt();
          if (iCurType != type) {
            jsData[u8"ColorEffect"] = type;
            VarBox::WriteSettings();
          }
          auto colList = jsData[u8"BackgroundColorRgba"].getArray() |
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
              .getValueInt();
        }}},
  };
  m_FuncStingMap = {
    {u8"AppSelectSkin", [this](std::u8string name) {
      VarBox::GetSettings(u8"FormGlobal")[u8"CurSkin"] = name;
      VarBox::WriteSettings();

      auto menu = m_ExMenus[u8"AppSelectSkin"];
      const QString qname = QString::fromUtf8(name.data(), name.size());
      for (auto i: menu->children()) {
        QAction* action = qobject_cast<QAction*>(i);
        action->setChecked(action->text() == qname);
      }

      menu = m_ExMenus[u8"AppRemoveSkin"];
      menu->clear();
      for (const auto& [key, _]: VarBox::GetSettings(u8"FormGlobal")[u8"UserSkins"].getObject()) {
        if (name == key) continue;
        auto action = menu->addAction(QString::fromUtf8(key.data(), key.size()));
        connect(action, &QAction::triggered, this, std::bind(m_FuncStingMap[u8"AppRemoveSkin"], name));
      }
      VarBox::GetSpeedBox()->UpdateSkin();
      VarBox::ShowMsg("更换皮肤成功！");
    }},
    {u8"AppRemoveSkin", [this](std::u8string name) {
      const QString qname = QString::fromUtf8(name.data(), name.size());
      auto& object = VarBox::GetSettings(u8"FormGlobal")[u8"UserSkins"];
      object.remove(name);
      VarBox::WriteSettings();

      auto menu = m_ExMenus[u8"AppSelectSkin"];
      const auto rmItem = [&qname](QMenu* menu) {
        const auto& lst = menu->children();
        QAction* action = qobject_cast<QAction*>(*std::find_if(lst.cbegin(), lst.cend(), [&qname](const QObject* item){
          return qobject_cast<const QAction*>(item)->text() == qname;
        }));
        menu->removeAction(action);
      };
      rmItem(menu);
      

      if (object.emptyO())
        menu->removeAction(
          qobject_cast<QAction*>(menu->children().back()));

      menu = m_ExMenus[u8"AppRemoveSkin"];
      rmItem(menu);

      VarBox::ShowMsg("删除皮肤成功！");
    }}
  };
}

bool NeoMenu::ChooseFolder(QString title, QString& current) {
  current = QFileDialog::getExistingDirectory(this, title, current);
  return !current.isEmpty() && QDir().exists(current);
}

void NeoMenu::GetMenuContent(QMenu* parent, const YJson& data) {
  for (const auto& [i, j] : data.getObject()) {
    const std::u8string& type = j[u8"type"].getValueString();
    if (type == u8"Separator") {
      parent->addSeparator();
      continue;
    }
    QAction* action = parent->addAction(QString::fromUtf8(i.data(), i.size()));
    if (auto iter = j.find(u8"tip"); iter != j.endO()) {
      const std::u8string_view tip = iter->second.getValueString();
      action->setToolTip(QString::fromUtf8(tip.data(), tip.size()));
    }
    if (type == u8"Normal") {
      const auto& function =
          m_FuncNormalMap[j[u8"function"].getValueString()];
      connect(action, &QAction::triggered, this, function);
    } else if (type == u8"Group") {
      QMenu* menu = new QMenu(parent);
      if (auto& name = j[u8"name"]; name.isString()) {
        m_ExMenus[name.getValueString()] = menu;
      }
      menu->setToolTipsVisible(true);
      menu->setAttribute(Qt::WA_TranslucentBackground, true);
      action->setMenu(menu);
      GetMenuContent(menu, j[u8"children"]);
    } else if (type == u8"Checkable") {

      action->setCheckable(true);
      if (j[u8"checked"].isNull()) {
        const auto fnName = j[u8"function"].getValueString();
        const auto& [m, n] = m_FuncCheckMap[fnName];
        m_CheckableActions[fnName] = action;
        action->setChecked(n());
        connect(action, &QAction::triggered, this, m);
      } else {
        action->setChecked(j[u8"checked"].isTrue());
        connect(action, &QAction::triggered,
         this, 
         std::bind(
          m_FuncStingMap[j[u8"function"].getValueString()], 
          j[u8"string"].getValueString()
        ));
      }
    } else if (type == u8"ExclusiveGroup") {
      const std::u8string& functionName = j[u8"function"].getValueString();
      const auto& functions = m_FuncItemCheckMap[functionName];
      QMenu* menu = new QMenu(parent);
      menu->setToolTipsVisible(true);
      menu->setAttribute(Qt::WA_TranslucentBackground, true);
      action->setMenu(menu);
      const auto& children = j[u8"children"];
      GetMenuContent(menu, children);

      const auto& range = j[u8"range"].getArray();
      const int first = range.front().getValueInt();
      int last = range.back().getValueInt();
      if (last == -1)
        last = static_cast<int>(children.getObject().size());
      int index = 0, check = functions.second();
      QActionGroup* group = new QActionGroup(menu);
      group->setExclusive(true);
      m_ExMenus[functionName] = menu;
      m_ExclusiveGroups[functionName] = group;

      for (auto& k : menu->actions()) {
        if (k->property("separator").toBool()) {
          menu->insertSeparator(k);
        }
        if (index < first) {
          continue;
        } else if (index >= last) {
          break;
        }
        connect(k, &QAction::triggered, this,
              std::bind(functions.first, index));
        group->addAction(k);
        k->setChecked(index == check);
        ++index;
      }
    } else if (type == u8"GroupItem") {
      action->setCheckable(true);
      action->setProperty("separator", j[u8"separator"].isTrue());
    } else if (type == u8"VarGroup") {
      QMenu* menu = new QMenu(parent);
      menu->setToolTipsVisible(true);
      menu->setAttribute(Qt::WA_TranslucentBackground, true);
      action->setMenu(menu);
      m_ExMenus[j[u8"name"].getValueString()] = menu;
    } else if (type == u8"StringItem") {
      const auto& function =
          m_FuncStingMap[j[u8"function"].getValueString()];
      const auto& string = j[u8"string"].getValueString();
      connect(action, &QAction::triggered, this, std::bind(
        function, string));
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
                 = path.u8string();
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
                                 .getValueString();
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
              jsApiData.find(name)->first = viewNewName;
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
                jsExInfo->find(u8"ApiData")->second[name][u8"Paths"]);
            if (changed) {
              LoadWallpaperExmenu();
            }
        });

        for (int32_t index = 0, cIndex = data[u8"CurPath"].getValueInt();
             auto& path : data[u8"Paths"].getArray()) {
          std::u8string_view path_view = path.getValueString();
          temp = tempMenu->addAction(
              QString::fromUtf8(path_view.data(), path_view.size()));
          temp->setCheckable(true);
          pSonGroup->addAction(temp);
          connect(temp, &QAction::triggered, this, [=](bool checked) {
            jsExInfo->find(u8"ApiData")->second[name][u8"CurPath"]
                 = index;
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
          auto& obj = jsExInfo->find(u8"ApiData")->second[viewKeyName];
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
          GetListWidget(QStringLiteral("参数输入"), QStringLiteral("请输入Api路径"), obj[u8"Paths"]);
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
                       ->second[u8TypeName][u8"Parameter"].getObject();
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
      YJson::swap(jsData, iter->second[u8"Parameter"]);
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

  // auto argview = data.getArray() | std::views::transform([](const YJson& item) {
  //    std::u8string_view str = item.getValueString();
  //    return QString::fromUtf8(str.data(), str.size());
  // });

  // pLstWgt->addItems(QStringList(argview.begin(), argview.end()));
  for (const auto& item: data.getArray()) {
    std::u8string_view str = item.getValueString();
    pLstWgt->addItem(QString::fromUtf8(str.data(), str.size()));
  }

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
    if (VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"].isFalse())
      VarBox::GetSpeedBox()->hide();
    pSystemTray->setIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
    pSystemTray->setContextMenu(this);
    pSystemTray->setToolTip("Neobox");
    connect(pSystemTray, &QSystemTrayIcon::activated,
      [](QSystemTrayIcon::ActivationReason reason){
        switch (reason)
        {
          case QSystemTrayIcon::DoubleClick:
            if (VarBox::GetSpeedBox()->isVisible()) {
              VarBox::GetSpeedBox()->hide();
              VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"] = false;
              VarBox::ShowMsg("隐藏悬浮窗成功！");
            } else {
              VarBox::GetSpeedBox()->show();
              VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"] = true;
              VarBox::ShowMsg("显示悬浮窗成功！");
            }
            VarBox::WriteSettings();
            break;
          case QSystemTrayIcon::Trigger:
            VarBox::GetSpeedBox()->raise();
            break;
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
