#include <nativeex.h>
#include <pluginobject.h>
#include <yjson.h>
#include <glbobject.h>

#include <QInputDialog>
#include <QActionGroup>
#include <QFileDialog>

namespace fs = std::filesystem;

NativeExMenu::NativeExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback),
  m_ActionGroup(new QActionGroup(this)),
  m_Separator(addSeparator())
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
      glb->glbShowMsg("取消设置成功。");
      return;
    }
    number = data;
    m_CallBack(true);
    glb->glbShowMsg("设置成功！");
  });
  connect(addAction("添加更多"), &QAction::triggered, this, &NativeExMenu::AddApi);
}

void NativeExMenu::LoadSubSettingMenu(QAction* action)
{
  QMenu* subMenu = new QMenu(this);
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
        m_CallBack(false);

        action->setChecked(true);
        delete action->menu();
        LoadSubSettingMenu(action);
        if (iter != actions.end()) {
          delete (*iter)->menu();
          LoadSubSettingMenu(*iter);
        }
        glb->glbShowMsg("修改成功！");
      });
    connect(subMenu->addAction("删除此项"),
      &QAction::triggered, this, [action, this, viewName]() {
        delete action->menu();
        m_ActionGroup->removeAction(action);
        delete action;
        m_Data[u8"dirs"].remove(viewName);
        m_CallBack(false);

        glb->glbShowMsg("修改成功！");
      });
  }

  connect(subMenu->addAction("位置选择"), &QAction::triggered, this, std::bind(&NativeExMenu::EditApi, this, action));

  auto const randomAction = subMenu->addAction("随机遍历");
  randomAction->setCheckable(true);
  randomAction->setChecked(m_Data[u8"dirs"][viewName][u8"random"].isTrue());
  connect(randomAction, &QAction::triggered, this, [this, viewName](bool on){
    m_Data[u8"dirs"][viewName][u8"random"] = on;
    m_CallBack(false);
    glb->glbShowMsg("设置成功！");
  });

  auto const recursionAcion = subMenu->addAction("递归遍历");
  recursionAcion->setCheckable(true);
  recursionAcion->setChecked(m_Data[u8"dirs"][viewName][u8"recursion"].isTrue());
  connect(recursionAcion, &QAction::triggered, this, [this, viewName](bool on){
    m_Data[u8"dirs"][viewName][u8"recursion"] = on;
    m_CallBack(false);
    glb->glbShowMsg("设置成功！");
  });
}

void NativeExMenu::EditApi(QAction* action)
{
  auto& u8dir = m_Data[u8"dirs"][PluginObject::QString2Utf8(action->text())][u8"imgdir"].getValueString();
  auto const newdir = QFileDialog::getExistingDirectory(this, "选择本地壁纸文件夹", PluginObject::Utf82QString(u8dir));

  if (newdir.isEmpty()) {
    glb->glbShowMsg("取消设置成功。");
    return;
  }

  fs::path path = newdir.toStdU16String();
  path.make_preferred();
  u8dir = path.u8string();

  m_CallBack(false);
  glb->glbShowMsg("设置成功！");
}

void NativeExMenu::RenameApi(QAction* action)
{
  const auto qKeyName = action->text();
  const auto qKeyNewName = QInputDialog::getText(this,
      QStringLiteral("文字输入"),
      QStringLiteral("请输入新名称"),
      QLineEdit::Normal, qKeyName
  );
  if (qKeyNewName.isEmpty() || qKeyName == qKeyNewName)
    return;
  auto const viewName = PluginObject::QString2Utf8(qKeyName);
  auto const viewNewName = PluginObject::QString2Utf8(qKeyNewName);
  auto& jsApiData = m_Data[u8"dirs"];
  auto iter = jsApiData.find(viewNewName);
  if (iter != jsApiData.endO())
    return;
  if (action->isChecked()) {
    m_Data[u8"curdir"] = viewNewName;
  }
  jsApiData.find(viewName)->first = viewNewName;
  m_CallBack(false);
  action->setText(qKeyNewName);

  glb->glbShowMsg("修改成功！");
}

void NativeExMenu::AddApi()
{
  const QString qKeyName = QInputDialog::getText(this,
      QStringLiteral("请输入文字"), 
      QStringLiteral("请输自定义昵称"),
      QLineEdit::Normal, QStringLiteral("例：风景图片"));
  if (qKeyName.isEmpty()) return;
  auto viewKeyName(PluginObject::QString2Utf8(qKeyName));
  auto& obj = m_Data[u8"dirs"][viewKeyName];
  if (!obj.isNull()) {
    glb->glbShowMsg("昵称不能重复！");
    return;
  }

  auto qsFolder = QFileDialog::getExistingDirectory(this, "选择壁纸存放文件夹");
  if (qsFolder.isEmpty()) {
    glb->glbShowMsg("取消成功。");
    return;
  }
  fs::path folder = qsFolder.toStdU16String();
  folder.make_preferred();     // nice
  obj = YJson(YJson::O {
    {u8"imgdir", folder.u8string()},
    {u8"random", true},
    {u8"recursion", false},
  });

  auto const action = new QAction(qKeyName, this);
  insertAction(m_Separator, action);
  action->setCheckable(true);
  m_ActionGroup->addAction(action);
  m_CallBack(false);
  LoadSubSettingMenu(action);

  glb->glbShowMsg("添加成功！");
}