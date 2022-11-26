#include <nativeex.h>
#include <pluginobject.h>
#include <yjson.h>

NativeExMenu::NativeExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback)
{
  LoadSettingMenu();
}

NativeExMenu::~NativeExMenu()
{
  //
}

void NativeExMenu::LoadSettingMenu()
{
  auto const action1 = addAction("递归遍历");
  action1->setCheckable(true);
  action1->setChecked(m_Data[u8"recursion"].isTrue());
  connect(action1, &QAction::triggered, this,
    [this](bool checked) {
    m_Data[u8"recursion"] = checked;
    m_CallBack(true);
  });
  auto const action2 = addAction("随机抽选");
  action2->setCheckable(true);
  action2->setChecked(m_Data[u8"random"].isTrue());
  connect(action2, &QAction::triggered, this, [this](bool checked) {
    m_Data[u8"random"] = checked;
    m_CallBack(true);
  });
}