#include <scriptex.h>
#include <pluginobject.h>
#include <listeditor.h>
#include <yjson.h>

#include <QFileDialog>

#include <filesystem>

namespace fs = std::filesystem;

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
}