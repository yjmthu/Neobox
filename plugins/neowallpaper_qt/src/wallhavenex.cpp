#include <wallhavenex.h>
#include <mapeditor.h>
#include <pluginobject.h>
#include <yjson.h>
#include <pluginmgr.h>


#include <QActionGroup>
#include <QFileDialog>
#include <QDesktopServices>
#include <QInputDialog>

#include <filesystem>
#include <regex>

namespace fs =std::filesystem;

WallhavenExMenu::WallhavenExMenu(YJson data, MenuBase* parent, Callback callback, std::function<const fs::path&()> getCurImg)
  : WallBaseEx(callback, std::move(data), parent)
  , GetCurImage(getCurImg)
  , m_ActionGroup(new QActionGroup(this))
  , m_Separator(addSeparator())
{
  m_ActionGroup->setExclusive(true);
  LoadWallpaperTypes();
  LoadMoreActions();
}

WallhavenExMenu::~WallhavenExMenu()
{
  //
}

std::string WallhavenExMenu::GetImageName(const std::filesystem::path& path)
{
  auto const fileName = path.stem().string();
  std::regex const pattern("wallhaven-([a-z0-9]{6})", std::regex::icase);
  std::smatch result;
  if (std::regex_search(fileName, result, pattern)) {
    return result.str(1);
  }
  return {};
}

void WallhavenExMenu::LoadWallpaperTypes()
{
  const std::u8string& curType = m_Data[u8"WallhavenCurrent"].getValueString();
  for (auto& [u8TypeName, data] :m_Data[u8"WallhavenApi"].getObject()) {
    QAction* const action = new QAction(QString::fromUtf8(u8TypeName.data(), u8TypeName.size()), this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    action->setChecked(u8TypeName == curType);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);
  }
}

void WallhavenExMenu::LoadSubSettingMenu(QAction* action)
{
  auto const pSonMenu = new MenuBase(this);
  action->setMenu(pSonMenu);
  const std::u8string viewName = PluginObject::QString2Utf8(action->text());

  connect(pSonMenu->addAction(action->isChecked() ? "刷新此项" : "启用此项"), &QAction::triggered, pSonMenu, [this, action]() {
    auto& current = m_Data[u8"WallhavenCurrent"];
    auto const curName = action->text();
    auto const lastName = PluginObject::Utf82QString(current.getValueString());

    action->setChecked(true);
    current.setText(PluginObject::QString2Utf8(curName));
    if (lastName != curName) {
      const auto actions = m_ActionGroup->actions();
      auto lastAction = *std::find_if(actions.begin(), actions.end(), [&lastName](QAction* a){return a->text() == lastName;});
      delete lastAction->menu();
      LoadSubSettingMenu(lastAction);
      delete action->menu();
      LoadSubSettingMenu(action);
    }
    SaveSettings();
    mgr->ShowMsg("设置成功！");
  });
  if (!action->isChecked()) {
    connect(pSonMenu->addAction("删除此项"), &QAction::triggered, pSonMenu, [pSonMenu, this, action](){
      m_Data[u8"WallhavenApi"].remove(PluginObject::QString2Utf8(action->text()));
      SaveSettings();
      action->deleteLater();
      pSonMenu->deleteLater();
      mgr->ShowMsg("删除配置成功！");
    });
  }
  connect(pSonMenu->addAction("参数设置"), &QAction::triggered, this,
    std::bind(&WallhavenExMenu::EditCurType, this,
    std::ref(m_Data[u8"WallhavenApi"][PluginObject::QString2Utf8(action->text())][u8"Parameter"])));
  connect(pSonMenu->addAction("起始页面"), &QAction::triggered, this, [this, viewName](){
    auto& page = m_Data[u8"WallhavenApi"][viewName][u8"StartPage"];
    auto i = QInputDialog::getInt(this, "输入数字", "请输入壁纸起始页面，每页最多有24张壁纸", page.getValueInt(), 1, 100);
    page = i;
    SaveSettings();
    mgr->ShowMsg("配置成功！");
  });
  connect(pSonMenu->addAction("存储路径"), &QAction::triggered, this, [this, viewName]() {
    fs::path curdir = m_Data[u8"WallhavenApi"][viewName][u8"Directory"].getValueString();
    QString qCurDir = QString::fromStdU16String(curdir.u16string());

    qCurDir = QFileDialog::getExistingDirectory(this, "请选择壁纸存放路径", qCurDir);
    if (qCurDir.isEmpty() || qCurDir.isNull()) {
      return;
    }
    curdir = qCurDir.toStdU16String();
    curdir.make_preferred();
    m_Data[u8"WallhavenApi"][viewName][u8"Directory"] = curdir.u8string();
    SaveSettings();
  });
}

void WallhavenExMenu::LoadMoreActions()
{
  connect(addAction("页数设置"), &QAction::triggered, this, [this](){
    auto& page = m_Data[u8"WallhavenApi"][u8"PageSize"];
    auto i = QInputDialog::getInt(this, "输入数字", "请输入每次缓存的最大页数，每页最多有24张壁纸", page.getValueInt(), 1, 5);
    page = i;
    SaveSettings();
    mgr->ShowMsg("配置成功！");
  });
  auto ptr = addAction("全局参数");
  ptr->setToolTip("对所有类型的壁纸都生效的参数");
  connect(ptr, &QAction::triggered, this,
    std::bind(&WallhavenExMenu::EditCurType, this,
      std::ref(m_Data[u8"Parameter"])));
  connect(addAction("添加更多"), &QAction::triggered, this, std::bind(&WallhavenExMenu::AddNewType, this));
  connect(addAction("关于壁纸"), &QAction::triggered, this, [this]() {
    const auto name = GetImageName(GetCurImage());
    if (name.size() != 6) return;
    QDesktopServices::openUrl(
      QString::fromStdString("https://wallhaven.cc/w/" + name));
  });
  connect(addAction("相似壁纸"), &QAction::triggered, this, [this]() {
    const auto name = GetImageName(GetCurImage());
    if (name.size() != 6) return;
    auto const url =
        "https://wallhaven.cc/search?q=like:" + name;
    QDesktopServices::openUrl(QString::fromStdString(url));
  });
}

void WallhavenExMenu::AddNewType()
{
  auto& apis = m_Data[u8"WallhavenApi"];
  QString qKeyName = QInputDialog::getText(this, "请输入类型名称", "新名称", QLineEdit::Normal, "新壁纸");
  if (qKeyName.isEmpty()) {
    return;
  }

  std::u8string u8KeyName = PluginObject::QString2Utf8(qKeyName);
  if (apis.find(u8KeyName) != apis.endO()) {
    return;
  }

  const QString folder = QFileDialog::getExistingDirectory(this, "选择壁纸文件夹");
  if (folder.isEmpty() || folder.isNull()) {
    return;
  }

  fs::path path = folder.toStdU16String();
  path.make_preferred();

  apis.append(YJson::O {
    {u8"Parameter", YJson::O {}},
    {u8"Directory", path.u8string()},
    {u8"StartPage", 1}
  }, u8KeyName);

  EditNewType(u8KeyName);
}

void WallhavenExMenu::EditNewType(std::u8string typeName)
{
  auto& params = m_Data[u8"WallhavenApi"][typeName][u8"Parameter"];
  MapEditor editor("编辑参数", params, [this, typeName, &params](bool changed, const YJson& data){
    if (!changed) {
      m_Data[u8"WallhavenApi"].removeByValO(typeName);
      mgr->ShowMsg("取消设置成功！");
      return;
    }

    if (data.emptyO()) {
      m_Data[u8"WallhavenApi"].removeByValO(typeName);
      mgr->ShowMsg("参数列表不能为空！");
      return;
    }
  
    params = data;
    auto const action = new QAction(PluginObject::Utf82QString(typeName), this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);

    SaveSettings();
    mgr->ShowMsg("配置成功！");
  });
  editor.exec();
}

void WallhavenExMenu::EditCurType(YJson& curJson)
{
   MapEditor("编辑参数", curJson, [this, &curJson](bool changed, const YJson& data){
    if (!changed) {
      mgr->ShowMsg("取消设置成功！");
      return;
    }
    if (data.emptyO()) {
      mgr->ShowMsg("参数列表不能为空！");
      return;
    }

    curJson = data;
    SaveSettings();
    mgr->ShowMsg("配置成功！");
  }).exec();
}
