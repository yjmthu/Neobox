#include <directapiex.h>
#include <pluginobject.h>
#include <listeditor.h>
#include <yjson.h>
#include <pluginmgr.h>

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>

#include <filesystem>

namespace fs = std::filesystem;

DirectApiExMenu::DirectApiExMenu(YJson data, MenuBase* parent, Callback callback)
  : WallBaseEx(callback, std::move(data), parent)
  , m_ActionGroup(new QActionGroup(this))
  , m_Separator(addSeparator())
{
  LoadSettingMenu();
}

DirectApiExMenu::~DirectApiExMenu()
{
}

void DirectApiExMenu::LoadSettingMenu()
{
  const std::u8string& key = m_Data[u8"ApiUrl"].getValueString();
  for (const auto& [name, data]: m_Data[u8"ApiData"].getObject()) {
    QAction* action = new QAction(QString::fromUtf8(name.data(), name.size()), this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    action->setChecked(name == key);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);
  }
  connect(addAction("添加更多"), &QAction::triggered, this, &DirectApiExMenu::AddApi);
}

void DirectApiExMenu::LoadSubSettingMenu(QAction* action)
{
  MenuBase* subMenu = new MenuBase(this);
  action->setMenu(subMenu);

  auto u8ViewName = PluginObject::QString2Utf8(action->text());

  connect(subMenu->addAction("修改名称"),
    &QAction::triggered, this, std::bind(&DirectApiExMenu::RenameApi, this, action));

  const std::u8string viewName = PluginObject::QString2Utf8(action->text());

  if (!action->isChecked()) {
    connect(subMenu->addAction("启用此项"),
      &QAction::triggered, this, [this, action, viewName]() {
        auto const actions = m_ActionGroup->actions();
        auto lastName = PluginObject::Utf82QString(m_Data[u8"ApiUrl"].getValueString());
        auto iter = std::find_if(actions.begin(), actions.end(), [&lastName](QAction* action){
          return action->text() == lastName;
        });

        m_Data[u8"ApiUrl"] = viewName;
        SaveSettings();

        action->setChecked(true);
        delete action->menu();
        LoadSubSettingMenu(action);
        if (iter != actions.end()) {
          delete (*iter)->menu();
          LoadSubSettingMenu(*iter);
        }
        mgr->ShowMsg("修改成功！");
      });
    connect(subMenu->addAction("删除此项"),
      &QAction::triggered, this, [action, this, viewName]() {
        delete action->menu();
        m_ActionGroup->removeAction(action);
        delete action;
        m_Data[u8"ApiData"].remove(viewName);
        SaveSettings();

        mgr->ShowMsg("修改成功！");
      });
  }

  connect(subMenu->addAction("域名编辑"), &QAction::triggered, this, [this, u8ViewName](){
    auto& u8CurDomain = m_Data[u8"ApiData"][u8ViewName][u8"Url"].getValueString();
    auto const qCurDomain = PluginObject::Utf82QString(u8CurDomain);

    const auto qNewDomain = QInputDialog::getText(this,
      QStringLiteral("文字输入"),
      QStringLiteral("请输入新域名"),
      QLineEdit::Normal, qCurDomain
    );
    if (qNewDomain.isEmpty() || qCurDomain == qNewDomain) {
      mgr->ShowMsg("取消成功");
      return;
    }

    u8CurDomain = PluginObject::QString2Utf8(qNewDomain);
    if (u8CurDomain.ends_with(u8"/")) {
      u8CurDomain.pop_back();
    }
    SaveSettings();
    mgr->ShowMsg("设置成功");
  });

  connect(subMenu->addAction("网址编辑"), &QAction::triggered, this, std::bind(&DirectApiExMenu::EditApi, this, action));

  connect(subMenu->addAction("存储路径"), &QAction::triggered, this, [this, u8ViewName]() {
    auto& u8CurDir = m_Data[u8"ApiData"][u8ViewName][u8"Directory"].getValueString();
    auto u8NewDir = GetExistingDirectory("选择存储壁纸的文件夹", u8CurDir);
    if(!u8NewDir) {
      mgr->ShowMsg("取消成功");
    } else {
      u8CurDir.swap(*u8NewDir);
      SaveSettings();
      mgr->ShowMsg("设置成功");
    }
  });

  subMenu->addSeparator();
  LoadPaths(subMenu, viewName);
}

void DirectApiExMenu::EditApi(QAction* action)
{
  auto& paths = m_Data[u8"ApiData"][PluginObject::QString2Utf8(action->text())][u8"Paths"];
  ListEditor editor(
    "列表编辑器", paths, [this, action, &paths](bool changed, const YJson& data){
      if (!changed) {
        mgr->ShowMsg("取消设置成功！");
        return;
      }

      if (data.emptyA()) {
        mgr->ShowMsg("列表不能为空！");
        return;
      }

      paths = data;
      SaveSettings();
      delete action->menu();
      LoadSubSettingMenu(action);
    });
  editor.m_ArgEditTitle = "输入该域名下的网址路径(path)";
  editor.m_ArgEditLabel = "路径：";
  editor.exec();
}

void DirectApiExMenu::RenameApi(QAction* action)
{
  const QString qKeyName = action->text();
  const QString qKeyNewName = QInputDialog::getText(this,
      QStringLiteral("文字输入"),
      QStringLiteral("请输入新名称"),
      QLineEdit::Normal, qKeyName
  );
  if (qKeyNewName.isEmpty() || qKeyName == qKeyNewName)
    return;
  const std::u8string viewName = PluginObject::QString2Utf8(qKeyName);
  const std::u8string viewNewName = PluginObject::QString2Utf8(qKeyNewName);
  auto& jsApiData = m_Data[u8"ApiData"];
  auto iter = jsApiData.find(viewNewName);
  if (iter != jsApiData.endO())
    return;
  if (action->isChecked()) {
    m_Data[u8"ApiUrl"] = viewNewName;
  }
  jsApiData.find(viewName)->first = viewNewName;
  SaveSettings();
  action->setText(qKeyNewName);

  mgr->ShowMsg("修改成功！");
}

void DirectApiExMenu::LoadPaths(MenuBase* subMenu, const std::u8string& name)
{
  QActionGroup* pSonGroup = new QActionGroup(subMenu);
  // for (const auto& [name, data] : m_Data[u8"ApiData"].getObject()) {
    auto& data = m_Data[u8"ApiData"][name];
    auto& curPath = data[u8"CurPath"];
    for (int32_t index = 0, cIndex = curPath.getValueInt();
          auto& path : data[u8"Paths"].getArray()) {
      const std::u8string& path_view = path.getValueString();
      QAction* action = subMenu->addAction(
          QString::fromUtf8(path_view.data(), path_view.size()));
      action->setCheckable(true);
      pSonGroup->addAction(action);
      connect(action, &QAction::triggered, this, [&curPath, this, index](bool checked) {
        curPath = index;
        SaveSettings();
      });
      action->setChecked(cIndex == index++);
    }
  // }
}

void DirectApiExMenu::AddApi()
{
  const QString qKeyName = QInputDialog::getText(this,
      QStringLiteral("请输入文字"), 
      QStringLiteral("请输入要添加的Api的名称"),
      QLineEdit::Normal, QStringLiteral("New Api"));
  if (qKeyName.isEmpty()) return;
  std::u8string viewKeyName(PluginObject::QString2Utf8(qKeyName));
  auto& obj = m_Data[u8"ApiData"][viewKeyName];
  if (!obj.isNull()) {
    mgr->ShowMsg("昵称不能重复！");
    return;
  }

  QString qDomain = QInputDialog::getText(this,
      QStringLiteral("请输入文字"), 
      QStringLiteral("请输入要添加的Api的域名"),
      QLineEdit::Normal, QStringLiteral("https://w.wallhaven.cc"));
  if (qDomain.isEmpty()) qDomain = QStringLiteral("https://w.wallhaven.cc");
  std::u8string u8Domain(PluginObject::QString2Utf8(qDomain));
  QString qsFolder = QFileDialog::getExistingDirectory(this, "选择壁纸存放文件夹");
  if (qsFolder.isEmpty()) {
    return;
  }
  fs::path folder = qsFolder.toStdU16String();
  folder.make_preferred();     // nice
  obj = YJson(YJson::O {
    {u8"Url", u8Domain},
    {u8"CurPath", 0},
    {u8"Paths", YJson::A { u8"/full/6o/wallhaven-6oxgp6.jpg" }},
    {u8"Directory", folder.u8string()},
    {u8"ImageNameFormat", u8"{0:%Y-%m-%d} {0:%H%M%S}.jpg"}
  });

  auto const action = new QAction(qKeyName, this);
  insertAction(m_Separator, action);
  action->setCheckable(true);
  m_ActionGroup->addAction(action);
  LoadSubSettingMenu(action);
  EditApi(action);
}
