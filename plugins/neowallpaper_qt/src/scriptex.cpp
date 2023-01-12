#include <scriptex.h>
#include <pluginobject.h>
#include <listeditor.h>
#include <yjson.h>
#include <pluginmgr.h>

#include <QFileDialog>
#include <QActionGroup>
#include <QInputDialog>

#include <filesystem>

namespace fs = std::filesystem;

ScriptExMenu::ScriptExMenu(YJson& data, MenuBase* parent, std::function<void(bool)> callback):
  MenuBase(parent),
  m_Data(data),
  m_CallBack(callback),
  m_ActionGroup(new QActionGroup(this)),
  m_Separator(addSeparator())
{
  LoadSettingMenu();
}

ScriptExMenu::~ScriptExMenu()
{
  //
}

void ScriptExMenu::LoadSettingMenu()
{
  const std::u8string& key = m_Data[u8"curcmd"].getValueString();
  for (const auto& [name, data]: m_Data[u8"cmds"].getObject()) {
    QAction* action = new QAction(QString::fromUtf8(name.data(), name.size()), this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    action->setChecked(name == key);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);
  }

  connect(addAction("添加更多"), &QAction::triggered, this, &ScriptExMenu::AddApi);
}

void ScriptExMenu::LoadSubSettingMenu(QAction* action)
{
  auto const subMenu = new MenuBase(this);
  action->setMenu(subMenu);

  connect(subMenu->addAction("修改名称"),
    &QAction::triggered, this, std::bind(&ScriptExMenu::RenameApi, this, action));

  const std::u8string viewName = PluginObject::QString2Utf8(action->text());

  if (!action->isChecked()) {
    connect(subMenu->addAction("启用此项"),
      &QAction::triggered, this, [this, action, viewName]() {
        auto const actions = m_ActionGroup->actions();
        auto& curdir = m_Data[u8"curcmd"];
        auto lastName = PluginObject::Utf82QString(curdir.getValueString());
        auto iter = std::find_if(actions.begin(), actions.end(), [&lastName](QAction* action){
          return action->text() == lastName;
        });

        curdir = viewName;
        m_CallBack(false);

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
        m_Data[u8"cmds"].remove(viewName);
        m_CallBack(false);

        mgr->ShowMsg("修改成功！");
      });
  }

  connect(subMenu->addAction("位置选择"), &QAction::triggered, this, std::bind(&ScriptExMenu::EditApi, this, action));

  connect(subMenu->addAction("命令编辑"), &QAction::triggered, this, [this, viewName](){
    auto& u8cmd = m_Data[u8"cmds"][viewName][u8"command"].getValueString();
    const auto qNewCmd = QInputDialog::getText(this,
        QStringLiteral("命令输入"),
        QStringLiteral("请输入新命令"),
        QLineEdit::Normal, PluginObject::Utf82QString(u8cmd)
    );
    if (qNewCmd.isEmpty()) {
      mgr->ShowMsg("取消设置成功。");
      return;
    }

    u8cmd = PluginObject::QString2Utf8(qNewCmd);
    m_CallBack(false);
    mgr->ShowMsg("设置成功！");
  });
}

void ScriptExMenu::RenameApi(QAction* action)
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
  auto& jsApiData = m_Data[u8"cmds"];
  auto iter = jsApiData.find(viewNewName);
  if (iter != jsApiData.endO())
    return;
  if (action->isChecked()) {
    m_Data[u8"curcmd"] = viewNewName;
  }
  jsApiData.find(viewName)->first = viewNewName;
  m_CallBack(false);
  action->setText(qKeyNewName);

  mgr->ShowMsg("修改成功！");
}

void ScriptExMenu::EditApi(QAction* action)
{
  auto& u8dir = m_Data[u8"cmds"][PluginObject::QString2Utf8(action->text())][u8"directory"].getValueString();
  auto const newdir = QFileDialog::getExistingDirectory(this, "选择本地壁纸文件夹", PluginObject::Utf82QString(u8dir));

  if (newdir.isEmpty()) {
    mgr->ShowMsg("取消设置成功。");
    return;
  }

  fs::path path = newdir.toStdU16String();
  path.make_preferred();
  u8dir = path.u8string();

  m_CallBack(false);
  mgr->ShowMsg("设置成功！");
}

void ScriptExMenu::AddApi()
{
  const QString qKeyName = QInputDialog::getText(this,
      QStringLiteral("请输入文字"), 
      QStringLiteral("请输入命令的昵称："),
      QLineEdit::Normal, QStringLiteral("例：壁纸爬虫"));
  
  if (qKeyName.isEmpty()) return;
  std::u8string viewKeyName(PluginObject::QString2Utf8(qKeyName));
  auto& obj = m_Data[u8"cmds"][viewKeyName];
  if (!obj.isNull()) {
    mgr->ShowMsg("昵称不能重复！");
    return;
  }

  auto const qCmd = QInputDialog::getText(this,
      QStringLiteral("请输入文字"), 
      QStringLiteral("请输入要添加的命令代码："),
      QLineEdit::Normal, QStringLiteral("python.exe \"scripts/getpic.py\""));
  
  if (qCmd.isEmpty()) {
    mgr->ShowMsg("取消设置成功！");
    return;
  }

  QString qsFolder = QFileDialog::getExistingDirectory(this, "选择壁纸存放文件夹");
  if (qsFolder.isEmpty()) {
    return;
  }

  fs::path folder = qsFolder.toStdU16String();
  folder.make_preferred();     // nice

  obj = YJson(YJson::O {
    {u8"command", PluginObject::QString2Utf8(qCmd)},
    {u8"directory", folder.u8string() },
  });

  auto const action = new QAction(qKeyName, this);
  insertAction(m_Separator, action);
  action->setCheckable(true);
  m_ActionGroup->addAction(action);
  LoadSubSettingMenu(action);
}
