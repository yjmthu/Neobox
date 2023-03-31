#include <nativeex.h>
#include <pluginobject.h>
#include <pluginmgr.h>
#include <yjson.h>

#include <QInputDialog>
#include <QActionGroup>
#include <QFileDialog>

NativeExMenu::NativeExMenu(YJson data, MenuBase* parent, Callback callback)
  : WallBaseEx(callback, std::move(data), parent)
  , m_ActionGroup(new QActionGroup(this))
  , m_Separator(addSeparator())
{
  LoadSettingMenu();
}

NativeExMenu::~NativeExMenu()
{
  //
}

void NativeExMenu::LoadSettingMenu()
{
  const std::u8string& key = m_Data[u8"curdir"].getValueString();
  for (const auto& [name, data]: m_Data[u8"dirs"].getObject()) {
    QAction* action = new QAction(QString::fromUtf8(name.data(), name.size()), this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    action->setChecked(name == key);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);
  }

  connect(addAction("缓存数量"), &QAction::triggered, this, [this](){
    auto& number = m_Data[u8"max"];
    int data = QInputDialog::getInt(this, "输入数字", "输入每次预先缓存的列表中壁纸的数量", number.getValueInt(), 20, 1000);
    if (number == data) {
      mgr->ShowMsg("取消设置成功。");
      return;
    }
    number = data;
    SaveSettings();
    mgr->ShowMsg("设置成功！");
  });
  connect(addAction("添加更多"), &QAction::triggered, this, &NativeExMenu::AddApi);
}

void NativeExMenu::LoadSubSettingMenu(QAction* action)
{
  auto const subMenu = new MenuBase(this);
  action->setMenu(subMenu);

  connect(subMenu->addAction("修改名称"),
    &QAction::triggered, this, std::bind(&NativeExMenu::RenameApi, this, action));

  const std::u8string viewName = PluginObject::QString2Utf8(action->text());

  if (!action->isChecked()) {
    connect(subMenu->addAction("启用此项"),
      &QAction::triggered, this, [this, action, viewName]() {
        auto const actions = m_ActionGroup->actions();
        auto& curdir = m_Data[u8"curdir"];
        auto lastName = PluginObject::Utf82QString(curdir.getValueString());
        auto iter = std::find_if(actions.begin(), actions.end(), [&lastName](QAction* action){
          return action->text() == lastName;
        });

        curdir = viewName;
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
        m_Data[u8"dirs"].remove(viewName);
        SaveSettings();

        mgr->ShowMsg("修改成功！");
      });
  }

  connect(subMenu->addAction("位置选择"), &QAction::triggered, this, std::bind(&NativeExMenu::EditApi, this, action));

  auto const randomAction = subMenu->addAction("随机遍历");
  randomAction->setCheckable(true);
  randomAction->setChecked(m_Data[u8"dirs"][viewName][u8"random"].isTrue());
  connect(randomAction, &QAction::triggered, this, [this, viewName](bool on){
    m_Data[u8"dirs"][viewName][u8"random"] = on;
    SaveSettings();
    mgr->ShowMsg("设置成功！");
  });

  auto const recursionAcion = subMenu->addAction("递归遍历");
  recursionAcion->setCheckable(true);
  recursionAcion->setChecked(m_Data[u8"dirs"][viewName][u8"recursion"].isTrue());
  connect(recursionAcion, &QAction::triggered, this, [this, viewName](bool on){
    m_Data[u8"dirs"][viewName][u8"recursion"] = on;
    SaveSettings();
    mgr->ShowMsg("设置成功！");
  });
}

void NativeExMenu::EditApi(QAction* action)
{
  auto& u8Dir = m_Data[u8"dirs"][PluginObject::QString2Utf8(action->text())][u8"imgdir"].getValueString();
  auto u8NewDir = GetExistingDirectory("选择本地壁纸文件夹", u8Dir);
  if (!u8NewDir) {
    mgr->ShowMsg("取消设置成功");
  } else {
    u8Dir.swap(*u8NewDir);
    SaveSettings();
    mgr->ShowMsg("设置成功");
  }
}

void NativeExMenu::RenameApi(QAction* action)
{
  auto const oldString = PluginObject::QString2Utf8(action->text());

  auto newString = GetNewU8String(
    "文字输入", "请输入新名称", action->text());
  if (!newString) return;
  

  auto& jsApiData = m_Data[u8"dirs"];
  auto iter = jsApiData.find(*newString);
  if (iter != jsApiData.endO()) {
    mgr->ShowMsg("不能使用重复昵称");
    return;
  }
  
  if (action->isChecked()) {
    m_Data[u8"curdir"] = *newString;
  }
  jsApiData.find(oldString)->first = oldString;
  SaveSettings();
  action->setText(QString::fromUtf8(newString->data(), newString->size()));

  mgr->ShowMsg("修改成功！");
}

void NativeExMenu::AddApi()
{
  auto keyName = GetNewU8String("请输入文字", "请输入自定义昵称", QStringLiteral("例：风景图片"));
  if (!keyName) {
    mgr->ShowMsg("取消成功。");
    return;
  }
  auto& obj = m_Data[u8"dirs"][*keyName];
  if (!obj.isNull()) {
    mgr->ShowMsg("昵称不能重复！");
    return;
  }

  auto const folder = GetExistingDirectory(u8"选择壁纸存放的文件夹", u8".");
  if (!folder) {
    mgr->ShowMsg("取消成功。");
    return;
  }
  obj = YJson(YJson::O {
    {u8"imgdir", *folder},
    {u8"random", true},
    {u8"recursion", false},
  });

  auto const action = new QAction(QString::fromUtf8(keyName->data(), keyName->size()), this);
  insertAction(m_Separator, action);
  action->setCheckable(true);
  m_ActionGroup->addAction(action);
  SaveSettings();
  LoadSubSettingMenu(action);

  mgr->ShowMsg("添加成功！");
}