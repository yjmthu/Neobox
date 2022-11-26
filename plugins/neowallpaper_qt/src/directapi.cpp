#include <directapiex.h>
#include <pluginobject.h>
#include <listeditor.h>
#include <yjson.h>

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>

#include <filesystem>

namespace fs = std::filesystem;

DirectApiExMenu::DirectApiExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback),
  m_ActionGroup(new QActionGroup(this)),
  m_Separator(addSeparator())
{
  LoadSettingMenu();
}

DirectApiExMenu::~DirectApiExMenu()
{
  //
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
}

void DirectApiExMenu::LoadSubSettingMenu(QAction* action)
{
  QMenu* subMenu = new QMenu(this);
  action->setMenu(subMenu);

  connect(subMenu->addAction("修改名称"),
    &QAction::triggered, this, std::bind(&DirectApiExMenu::RenameApi, this, action));

  if (!action->isChecked()) {
    const std::u8string viewName = PluginObject::QString2Utf8(action->text());
    connect(subMenu->addAction("启用此项"),
      &QAction::triggered, this, [this, viewName]() {
        m_Data[u8"ApiUrl"] = viewName;
        m_CallBack(false);
        // LoadWallpaperExmenu();
      });
    connect(subMenu->addAction("删除此项"),
      &QAction::triggered, this, [action, this, viewName]() {
        delete action->menu();
        m_ActionGroup->removeAction(action);
        delete action;
        m_Data[u8"ApiData"].remove(viewName);
        m_CallBack(false);
      });
  }

  connect(subMenu->addAction("路径编辑"), &QAction::triggered, this, std::bind(&DirectApiExMenu::EditApi, this, action));
  subMenu->addSeparator();
  LoadPaths(subMenu);

  connect(subMenu->addAction("添加更多"), &QAction::triggered, this, &DirectApiExMenu::AddApi);
}

void DirectApiExMenu::EditApi(QAction* action)
{
  auto const editor = new ListEditor(
    "列表编辑器", m_Data[u8"ApiData"][PluginObject::QString2Utf8(action->text())][u8"Paths"], [this](){
      m_CallBack(false);
      /*LoadWallpaperExmenu();*/
    });
    editor->m_ArgEditTitle = "输入该域名下的网址路径(path)";
    editor->m_ArgEditLabel = "路径：";
  editor->show();
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
  m_CallBack(false);
  action->setText(qKeyNewName);
}

void DirectApiExMenu::LoadPaths(QMenu* subMenu)
{
  QActionGroup* pSonGroup = new QActionGroup(subMenu);
  for (const auto& [name, data] : m_Data[u8"ApiData"].getObject()) {
    auto& curPath = m_Data[u8"ApiData"][name][u8"CurPath"];
    for (int32_t index = 0, cIndex = data[u8"CurPath"].getValueInt();
          auto& path : data[u8"Paths"].getArray()) {
      const std::u8string& path_view = path.getValueString();
      QAction* action = subMenu->addAction(
          QString::fromUtf8(path_view.data(), path_view.size()));
      action->setCheckable(true);
      pSonGroup->addAction(action);
      connect(action, &QAction::triggered, this, [&curPath, this, index](bool checked) {
        curPath = index;
        m_CallBack(false);
      });
      action->setChecked(cIndex == index++);
    }
  }
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
