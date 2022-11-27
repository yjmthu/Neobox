#include <neospeedboxplg.h>
#include <speedbox.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>

#include <QMenu>
#include <QActionGroup>
#include <QInputDialog>
#include <QFileDialog>
#include <QFontDatabase>

#include <Windows.h>
#include <filesystem>

#define CLASS_NAME NeoSpeedboxPlg
#include <pluginexport.cpp>

namespace fs = std::filesystem;

NeoSpeedboxPlg::NeoSpeedboxPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neospeedboxplg", u8"网速悬浮")
{
  LoadFonts();
  InitFunctionMap();
}

NeoSpeedboxPlg::~NeoSpeedboxPlg()
{
  //
}

void NeoSpeedboxPlg::InitFunctionMap() {
  m_FunctionMapVoid = {
    {u8"enableBlur", {
      u8"模糊背景", u8"Windows10+或KDE下的模糊效果", [this](){
        //  HWND hWnd = reinterpret_cast<HWND>(
        //      m_Speedbox->winId());
        // QColor col(Qt::transparent);
        //  SetWindowCompositionAttribute(
        //      hWnd, static_cast<ACCENT_STATE>(iCurType),
        //      qRgba(col.blue(), col.green(), col.red(), col.alpha()));
         glb->glbShowMsg("设置颜色成功！");
      }
    }},
    {u8"moveLeftTop", {
      u8"还原位置", u8"将窗口移动到左上方位置", std::bind(&SpeedBox::InitMove, m_Speedbox)
    }}
  };
}

void NeoSpeedboxPlg::InitMenuAction(QMenu* pluginMenu)
{
  auto action = pluginMenu->addAction("网卡选择");
  auto menu = new QMenu(pluginMenu);
  action->setMenu(menu);
  m_Speedbox = new SpeedBox(m_Settings, menu);
  AddMainObject(m_Speedbox);   // 添加到对象列表
  this->PluginObject::InitMenuAction(pluginMenu);
  pluginMenu->addSeparator();
  LoadChooseSkinMenu(pluginMenu);
  pluginMenu->addAction("皮肤选择")->setMenu(m_ChooseSkinMenu);
  AddSkinConnect(pluginMenu->addAction("添加皮肤"));
  LoadRemoveSkinMenu(pluginMenu);
  pluginMenu->addAction("皮肤删除")->setMenu(m_RemoveSkinMenu);

  m_Speedbox->InitShow();
}

YJson& NeoSpeedboxPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) return settings;
  return settings = YJson::O {
    {u8"ShowForm", true},
    {u8"Position", YJson::A {100, 100 }},
    {u8"ShowTrayIcon", true,},
    {u8"ColorEffect", 0,},
    {u8"BackgroundColorRgba", YJson::A { 51, 51, 119, 204 } },
    {u8"CurSkin", u8"经典火绒"},
    {u8"UserSkins", YJson::O {
      {u8"经典火绒", u8":/skins/Huorong.ui"},
      {u8"电脑管家", u8":/skins/Guanjia.ui"},
      {u8"数字卫士", u8":/skins/360.ui"},
      {u8"独霸一方", u8":/skins/duba.ui"}
    }},
    {u8"NetCardDisabled", YJson::A {}},
  };
  // we may not need to call SaveSettings;
}

void NeoSpeedboxPlg::LoadRemoveSkinMenu(QMenu* parent)
{
  m_RemoveSkinMenu = new QMenu(parent);
  const std::u8string& curSkin = m_Settings[u8"CurSkin"].getValueString();
  for (const auto& [name, _]: m_Settings[u8"UserSkins"].getObject()) {
    RemoveSkinConnect(m_RemoveSkinMenu->addAction(QString::fromUtf8(name.data(), name.size())));
  }
}

void NeoSpeedboxPlg::LoadChooseSkinMenu(QMenu* parent)
{
  m_ChooseSkinMenu = new QMenu(parent);
  m_ChooseSkinGroup = new QActionGroup(m_ChooseSkinMenu);

  const std::u8string& curSkin = m_Settings[u8"CurSkin"].getValueString();
  for (const auto& [name, _]: m_Settings[u8"UserSkins"].getObject()) {
    QAction* action = m_ChooseSkinMenu->addAction(PluginObject::Utf82QString(name));
    const QString qname = action->text();
    action->setCheckable(true);
    m_ChooseSkinGroup->addAction(action);
    action->setChecked(curSkin == name);
    ChooseSkinConnect(action);
  }
}

void NeoSpeedboxPlg::RemoveSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, m_RemoveSkinMenu, [action, this](){
    const QString qname = action->text();
    const std::u8string name = PluginObject::QString2Utf8(qname);
    if (name == m_Settings[u8"CurSkin"].getValueString()) {
      glb->glbShowMsg("当前皮肤正在使用，无法删除！");
      return;
    }
    m_Settings[u8"UserSkins"].remove(name);
    mgr->SaveSettings();
    m_RemoveSkinMenu->removeAction(action);

    QAction* anotherAction = nullptr;
    for (auto i: m_ChooseSkinGroup->actions()) {
      if (i->text() == qname) continue;
      anotherAction = i;
      break;
    }
    if (anotherAction) {
      m_ChooseSkinGroup->removeAction(anotherAction);
      m_ChooseSkinMenu->removeAction(anotherAction);
    }
    glb->glbShowMsg("删除皮肤成功！");
  });
}

void NeoSpeedboxPlg::ChooseSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, m_ChooseSkinMenu, [action, this](){
    m_Settings[u8"CurSkin"] = PluginObject::QString2Utf8(action->text());
    mgr->SaveSettings();
    m_Speedbox->UpdateSkin();
    glb->glbShowMsg("更换皮肤成功！");
  });
}

void NeoSpeedboxPlg::AddSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, [action, this](){
      const QString qname = action->text();
      const QString qSkinName = QInputDialog::getText(glb->glbGetMenu(), "输入", "请输入壁纸名字：");
      if (qSkinName.isEmpty() || qSkinName.isNull()) {
        glb->glbShowMsg("添加皮肤失败！");
        return;
      }
      for (auto qobj: m_ChooseSkinMenu->children()) {
        if (qobject_cast<QAction*>(qobj)->text() == qSkinName) {
          glb->glbShowMsg("请勿输入已经存在的名称！");
          return;
        }
      }

      const QString qFilePath = QFileDialog::getOpenFileName(glb->glbGetMenu(), "选择文件", ".", "(*.ui)");
      if (qFilePath.isEmpty() || !QFile::exists(qFilePath)) {
        glb->glbShowMsg("添加皮肤失败！");
        return;
      }
      const fs::path path = PluginObject::QString2Utf8(qFilePath);
      const std::u8string u8FilePath = u8"styles/" + path.filename().u8string();

      m_Settings[u8"UserSkins"].append(u8FilePath, PluginObject::QString2Utf8(qSkinName));
      mgr->SaveSettings();

      auto action = m_ChooseSkinMenu->addAction(qSkinName);
      action->setToolTip(QString::fromUtf8(u8FilePath.data(), u8FilePath.size()));
      action->setCheckable(true);
      ChooseSkinConnect(action);

      action = m_RemoveSkinMenu->addAction(qSkinName);
      RemoveSkinConnect(action);
      glb->glbShowMsg("添加皮肤成功！");
  });
}

void NeoSpeedboxPlg::LoadFonts() {
  QFontDatabase::addApplicationFont(
      QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
  QFontDatabase::addApplicationFont(
      QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}
