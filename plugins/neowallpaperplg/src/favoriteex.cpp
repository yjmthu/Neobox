#include <favoriteex.h>
#include <pluginobject.h>
#include <yjson.h>
#include <pluginmgr.h>

#include <QInputDialog>
#include <QActionGroup>
#include <QFileDialog>

FavoriteExMenu::FavoriteExMenu(YJson data, MenuBase* parent, Callback callback)
  : WallBaseEx(callback, std::move(data), parent)
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
    auto u8NewDir = GetExistingDirectory("选择存放壁纸的文件夹", u8CurDir);
    if (!u8NewDir) {
      mgr->ShowMsg("取消成功");
    } else {
      u8CurDir.swap(*u8NewDir);
      mgr->ShowMsg("设置成功");
      SaveSettings();
    }
  });

  auto const actionRandom = addAction("随机遍历");
  actionRandom->setCheckable(true);
  actionRandom->setChecked(m_Data[u8"Random"].isTrue());
  connect(actionRandom, &QAction::triggered, this, [this](bool on) {
    m_Data[u8"Random"] = on;
    mgr->ShowMsg("设置成功");
    SaveSettings();
  });
}
