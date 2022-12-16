#include <scriptex.h>
#include <pluginobject.h>
#include <listeditor.h>
#include <neoapp.h>
#include <yjson.h>

#include <QFileDialog>
#include <QActionGroup>

#include <filesystem>

extern GlbObject* glb;

// namespace fs = std::filesystem;

ScriptExMenu::ScriptExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback)
{
  LoadSettingMenu();
}

ScriptExMenu::~ScriptExMenu()
{
  //
}

void ScriptExMenu::LoadSettingMenu()
{
 #if 0
  connect(addAction("程序路径"), &QAction::triggered, this, [this]() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "请选择可执行文件", QString(), "*.*");
    if (fileName.isEmpty() || !QFile::exists(fileName))
      return;
    fs::path exe = PluginObject::QString2Utf8(fileName);
    exe.make_preferred();
    m_Data[u8"executeable"] = exe.u8string();
    m_CallBack(true);
  });
  connect(addAction("参数列表"), &QAction::triggered, this, [this](){
  auto const editor = new ListEditor(
    "列表编辑器", m_Data[u8"arglist"], [this](){
      m_CallBack(true);
      /*LoadWallpaperExmenu();*/
    });
    editor->m_ArgEditTitle = "输入命令行参数";
    editor->m_ArgEditLabel = "逐个写出可执行文件接受的参数：";
  editor->show();
  });
#endif
  LoadTypeChooseMenu();
}

void ScriptExMenu::LoadTypeChooseMenu()
{
  auto const action = addAction("选择来源");
  auto const menu = new QMenu(this);
  auto const group = new QActionGroup(menu);
  menu->setToolTipsVisible(true);
  menu->setAttribute(Qt::WA_TranslucentBackground);
  action->setMenu(menu);
  const std::u8string& curType = m_Data[u8"curcmd"].getValueString();
  for (const auto& info: m_Data[u8"cmds"].getObject()) {
    QString const name = PluginObject::Utf82QString(info.first);
    auto action = menu->addAction(name);
    action->setCheckable(true);
    group->addAction(action);
    action->setChecked(info.first == curType);
    connect(action, &QAction::triggered, menu, [action, this](){
      std::u8string name = PluginObject::QString2Utf8(action->text());
      m_Data[u8"curcmd"] = name;
      m_CallBack(true);
      glb->glbShowMsg("修改成功！");
    });
  }
}

void ScriptExMenu::LoadDataEditMenu()
{
  addAction("修改命令");
}