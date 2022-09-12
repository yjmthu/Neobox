#include <neomenu.h>
#include <screenfetch.h>
#include <sysapi.h>
#include <translatedlg.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>
#include <speedbox.h>
#include <shortcut.h>

#include <map>
#include <regex>
#include <vector>
#include <ranges>

#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#ifdef _WIN32
#include <Windows.h>
#elif def __linux__
#else
#endif

extern QString QImage2Text(const QImage& qImage);

NeoMenu::NeoMenu(QWidget* parent)
    : QMenu(parent),
      m_Shortcut(new Shortcut(this->parent())),
      m_TranslateDlg(new TranslateDlg(this)),
      m_Wallpaper(new Wallpaper(&VarBox::GetSettings(u8"Wallpaper"),
                                &VarBox::WriteSettings))
{
  setAttribute(Qt::WA_TranslucentBackground, true);
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
      {u8"AppOpenConfigDir",
       std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath()))},
      {u8"AppFormBackground",
        [this](){
          auto& jsData = VarBox::GetSettings(u8"FormGlobal");
          int iCurType = jsData[u8"ColorEffect"].second.getValueInt();
          auto colList = jsData[u8"BackgroundColorRgba"].second.getArray() | std::views::transform([](const YJson& i){return i.getValueInt();});
          auto colVector = std::vector<int>(colList.begin(), colList.end());
          const QColor col = QColorDialog::getColor(
              QColor(colVector[0], colVector[1], colVector[2], colVector[3]),
              this, "选择颜色", QColorDialog::ShowAlphaChannel);
          if (!col.isValid()) return;
          jsData[u8"BackgroundColorRgba"].second.getArray() = { col.red(), col.green(), col.blue(), col.alpha() };
          VarBox::WriteSettings();

          HWND hWnd = reinterpret_cast<HWND>(qobject_cast<const QWidget*>(parent())->winId());
          SetWindowCompositionAttribute(hWnd, static_cast<ACCENT_STATE>(iCurType),
              qRgba(col.blue(), col.green(), col.red(), col.alpha()));
        }
      },
      {u8"ToolOcrGetScreen",
       [this]() {
         QImage image;
         ScreenFetch* box = new ScreenFetch(image);
         QEventLoop loop;
         connect(box, &ScreenFetch::destroyed, &loop, &QEventLoop::quit);
         box->showFullScreen();
         loop.exec();
         m_TranslateDlg->Show(qobject_cast<const QWidget*>(parent())->frameGeometry(), QImage2Text(image));
       }},
      {u8"ToolOcrDataPath",
        [this](){
          std::u8string& u8Path = VarBox::GetSettings(u8"Tools")[u8"Ocr.TessdataDir"].second.getValueString();
          QByteArray folder = QFileDialog::getExistingDirectory(this, "请选择Tessdata数据文件存放位置", QString::fromUtf8(u8Path.data(), u8Path.size())).toUtf8();
          std::u8string u8NewPath = std::filesystem::path(std::u8string(folder.begin(), folder.end())).u8string();
          if (!u8NewPath.empty() && u8NewPath != u8Path) {
            u8Path.swap(u8NewPath);
            VarBox::WriteSettings();
          }
        }
      },
      {u8"ToolTransShowDlg", [this](){m_TranslateDlg->Show(qobject_cast<const QWidget*>(parent())->frameGeometry());}},
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
             QString::fromStdWString(m_Wallpaper->GetCurIamge().wstring());
         if (!ChooseFolder("选择壁纸文件夹", current))
           return;
         m_Wallpaper->SetCurDir(current.toStdWString());
       }},
      {u8"WallpaperTimeInterval",
       [this]() {
         int iNewTime =
             QInputDialog::getInt(this, "输入时间间隔", "时间间隔（分钟）：",
                                  m_Wallpaper->GetTimeInterval(), 5);
         if (iNewTime < 5) return;
         m_Wallpaper->SetTimeInterval(iNewTime);
       }},
      {u8"WallpaperClean", std::bind(&Wallpaper::ClearJunk, m_Wallpaper)},
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
      {u8"ToolTransRegistKey", {
        [this](bool checked){
          auto& settings = VarBox::GetSettings(u8"Tools");
          std::u8string& shortcut = settings[u8"Translate.Shortcut"].second.getValueString();
          QString qsShortcut = QString::fromUtf8(shortcut.data(), shortcut.size());
          if (checked) {
            m_Shortcut->RegistHotKey(qsShortcut, [this](){
              if (m_TranslateDlg->isVisible()) {
                m_TranslateDlg->hide();
              } else {
                m_TranslateDlg->Show(qobject_cast<QWidget*>(parent())->frameGeometry());
              }
            });
          } else {
            m_Shortcut->UnregistHotKey(qsShortcut);
          }
          settings[u8"Translate.RegisterHotKey"].second = checked;
          VarBox::WriteSettings();
        },
        [this]()->bool{
          auto& settings = VarBox::GetSettings(u8"Tools");
          bool regist = settings[u8"Translate.RegisterHotKey"].second.isTrue();
          std::u8string& shortcut = settings[u8"Translate.Shortcut"].second.getValueString();
          QString qsShortcut = QString::fromUtf8(shortcut.data(), shortcut.size());
          if (regist && !m_Shortcut->IsKeyRegisted(qsShortcut)) {
            m_Shortcut->RegistHotKey(qsShortcut, [this](){
              if (m_TranslateDlg->isVisible()) {
                m_TranslateDlg->hide();
              } else {
                m_TranslateDlg->Show(qobject_cast<QWidget*>(parent())->frameGeometry());
              }
            });
          }
          return regist;
        }}
      }
  };

  m_FuncItemCheckMap = {
    {u8"WallpaperType",
       {[this](int type) {
          m_Wallpaper->SetImageType(type);
          LoadWallpaperExmenu();
        },
        std::bind(&Wallpaper::GetImageType, m_Wallpaper)}},
    {u8"AppFormEffect",
      {[this](int type) {
        auto& jsData = VarBox::GetSettings(u8"FormGlobal");
        int iCurType = jsData[u8"ColorEffect"].second.getValueInt();
        if (iCurType != type) {
          jsData[u8"ColorEffect"].second = type;
          VarBox::WriteSettings();
        }
        auto colList = jsData[u8"BackgroundColorRgba"].second.getArray() | std::views::transform([](const YJson& i){return i.getValueInt();});
        auto colVector = std::vector<int>(colList.begin(), colList.end());
        HWND hWnd = reinterpret_cast<HWND>(qobject_cast<const QWidget*>(parent())->winId());
        SetWindowCompositionAttribute(hWnd, static_cast<ACCENT_STATE>(type),
            qRgba(colVector[2], colVector[1], colVector[0], colVector[3]));
      }, []()->int{
        return VarBox::GetSettings(u8"FormGlobal")[u8"ColorEffect"].second.getValueInt();
      }}
    }
  };
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
        QAction* action = pMainMenu->addAction(
            QString::fromUtf8(u8TypeName.data(), u8TypeName.size()));
        action->setCheckable(true);
        action->setChecked(
            u8TypeName ==
            jsExInfo->find(u8"WallhavenCurrent")->second.getValueString());
        group->addAction(action);
        QMenu* pSonMenu = new QMenu(pMainMenu);
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
      connect(temp, &QAction::triggered, this, [this, jsExInfo](){
        auto& u8ImgFmt = jsExInfo->find(u8"imgfmt")->second.getValueString();
        QString qImgFmt = QString::fromUtf8(u8ImgFmt.data(), u8ImgFmt.size());
        QByteArray&& qImgFmtNewByte = QInputDialog::getText(this, "图片名称", "输入名称格式", QLineEdit::Normal, qImgFmt).toUtf8();
        std::u8string u8ImgFmtNew(qImgFmtNewByte.begin(), qImgFmtNewByte.end());
        if (u8ImgFmtNew.empty() || u8ImgFmtNew == u8ImgFmt) return;
        u8ImgFmt.swap(u8ImgFmtNew);
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction("地理位置");
      connect(temp, &QAction::triggered, this, [this, jsExInfo](){
        auto& u8Mkt = jsExInfo->find(u8"mkt")->second.getValueString();
        QString qMkt = QString::fromUtf8(u8Mkt.data(), u8Mkt.size());
        QByteArray&& qMktNewByte = QInputDialog::getText(this, "图片地区", "输入图片所在地区", QLineEdit::Normal, qMkt).toUtf8();
        std::u8string u8MktNew(qMktNewByte.begin(), qMktNewByte.end());
        if (u8MktNew.empty() || u8MktNew == u8Mkt) return;
        u8Mkt.swap(u8MktNew);
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction("自动下载");
      temp->setCheckable(true);
      temp->setChecked(jsExInfo->find(u8"auto-download")->second.isTrue());
      connect(temp, &QAction::triggered, this, [this, jsExInfo](bool checked){
        jsExInfo->find(u8"auto-download")->second = checked;
        m_Wallpaper->m_Wallpaper->SetJson(true);
      });
      temp = pMainMenu->addAction("关于壁纸");
      connect(temp, &QAction::triggered, this, [jsExInfo](){
        size_t index = jsExInfo->find(u8"index")->second.getValueInt();
        auto link = jsExInfo->find(u8"images")->second[index][u8"copyrightlink"].second.getValueString() + u8"&filters=HpDate:\"20220904_1600\"";
        QDesktopServices::openUrl(QString::fromUtf8(link.data(), link.size()));
      });
      break;
    }
    case WallBase::NATIVE: {
}
    default:
      break;
  }
  connect(pMainMenu->addAction("打开位置"), &QAction::triggered, this, [this](){
      std::wstring args = L"/select, " + m_Wallpaper->GetCurIamge().wstring();
      ShellExecuteW(nullptr, L"open", L"explorer", args.c_str(), NULL, SW_SHOWNORMAL);
  });
}

void NeoMenu::WallhavenParams(QAction* action) {
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
          [pTable]() { pTable->insertRow(pTable->currentRow()); });
  connect(arButtons[1], &QPushButton::clicked, this,
          [pTable]() { pTable->removeRow(pTable->currentRow()); });
  connect(arButtons[2], &QPushButton::clicked, this, [=]() {
    YJson jsData = YJson::Object;
    for (int i = 0; i < pTable->rowCount(); ++i) {
      auto qsKeyArray = pTable->item(i, 0)->text().toUtf8();
      auto qsValueArray = pTable->item(i, 1)->text().toUtf8();
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
}
