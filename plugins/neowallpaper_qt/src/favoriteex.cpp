#include <favoriteex.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>

#include <QInputDialog>
#include <QActionGroup>
#include <QFileDialog>

namespace fs = std::filesystem;

FavoriteExMenu::FavoriteExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback)
{
  LoadSettingMenu();
}

FavoriteExMenu::~FavoriteExMenu()
{
  //
}

void FavoriteExMenu::LoadSettingMenu()
{
  connect(addAction("存储路径"), &QAction::triggered, this, [this]() {
    auto& u8CurDir = m_Data[u8"Directory"].getValueString();
    auto const qNewDir = QFileDialog::getExistingDirectory(
      glb->glbGetMenu(),
      QStringLiteral("选择存储壁纸的文件夹"),
      PluginObject::Utf82QString(u8CurDir)
    );
    if(qNewDir.isEmpty()) {
      glb->glbShowMsg("取消成功");
      return;
    }
    fs::path path = qNewDir.toStdU16String();
    path.make_preferred();
    
    if (path.u8string() == u8CurDir) {
      glb->glbShowMsg("取消成功");
      return;
    }
    u8CurDir = path.u8string();
    glb->glbShowMsg("设置成功");
    m_CallBack(true);
  });

  auto const actionRandom = addAction("随机遍历");
  actionRandom->setCheckable(true);
  actionRandom->setChecked(m_Data[u8"Random"].isTrue());
  connect(actionRandom, &QAction::triggered, this, [this](bool on) {
    m_Data[u8"Random"] = on;
    glb->glbShowMsg("设置成功");
    m_CallBack(true);
  });
}
