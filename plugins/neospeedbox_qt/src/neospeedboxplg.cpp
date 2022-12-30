#include <neospeedboxplg.h>
#include <speedbox.h>
#include <pluginobject.h>
#include <yjson.h>
#include <systemapi.h>
#include <neosystemtray.h>
#include <neoapp.h>

#include <QMenu>
#include <QActionGroup>
#include <QInputDialog>
#include <QFileDialog>
#include <QFontDatabase>
#include <QDropEvent>
#include <QMimeData>

#include <Windows.h>
#include <filesystem>
#include <ranges>

#define CLASS_NAME NeoSpeedboxPlg
#include <pluginexport.cpp>

namespace fs = std::filesystem;
using namespace std::literals;

NeoSpeedboxPlg::NeoSpeedboxPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neospeedboxplg", u8"网速悬浮")
{
  LoadFonts();
  InitFunctionMap();
}

NeoSpeedboxPlg::~NeoSpeedboxPlg()
{
  auto& followers = glb->glbGetSystemTray()->m_Followers;
  followers.erase(&m_ActiveWinodow);
  delete m_Speedbox;
}

void NeoSpeedboxPlg::InitFunctionMap() {
  m_PluginMethod = {
    {u8"moveLeftTop", {
      u8"还原位置", u8"将窗口移动到左上方位置", [this](PluginEvent, void*){
        m_Speedbox->InitMove();
      }, PluginEvent::Void}
    },
    {u8"enableBlur", {
      u8"模糊背景", u8"Windows10+或KDE下的模糊效果", [this](PluginEvent event, void* data){
        static bool firstRun = true;
        if (event == PluginEvent::Bool) {
          m_Settings[u8"ColorEffect"] = *reinterpret_cast<bool *>(data);
          auto status = *reinterpret_cast<bool *>(data) ? ACCENT_ENABLE_BLURBEHIND : ACCENT_DISABLED;
          auto hWnd = reinterpret_cast<HWND>(m_Speedbox->winId());
          const QColor col(Qt::transparent);
          SetWindowCompositionAttribute(hWnd, status,
            qRgba(col.blue(), col.green(), col.red(), col.alpha()));
          mgr->SaveSettings();
          if (!firstRun) glb->glbShowMsg("设置颜色成功！");
          firstRun = false;
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool *>(data) = m_Settings[u8"ColorEffect"].isTrue();
        }
      }, PluginEvent::Bool}
    },
  };

  m_ActiveWinodow = [this](PluginEvent event, void*){
    if (event == PluginEvent::MouseDoubleClick) {
      if (m_Speedbox->isVisible()) {
        m_Speedbox->hide();
        glb->glbShowMsg("隐藏悬浮窗成功！");
      } else {
        m_Speedbox->show();
        glb->glbShowMsg("显示悬浮窗成功！");
      }
    } else if (event == PluginEvent::MouseClick) {
      m_Speedbox->raise();
    }
  };

  static std::function setDropSkin = [this](PluginEvent event, void*data){
    if (event == PluginEvent::Drop) {
      const auto mimeData = reinterpret_cast<QDropEvent*>(data)->mimeData();
      if (!mimeData->hasUrls()) return;
      auto urls = mimeData->urls();
      auto uiUrlsView = urls | std::views::filter([](const QUrl& i) {
          return i.isValid() && i.isLocalFile() && i.fileName().endsWith(".ui");
        }) | std::views::transform([](const QUrl& url){
          return fs::path(PluginObject::QString2Utf8(url.toLocalFile()));
        }) | std::views::filter([](const fs::path& url) {
          return fs::exists(url) && url.has_filename();
        });
      
      std::vector<fs::path> vec(uiUrlsView.begin(), uiUrlsView.end());

      for (auto& file: vec) {
        auto const msg = u8"请输入皮肤“" + file.filename().u8string() + u8"”的昵称";
        const auto qSkinName = QInputDialog::getText(glb->glbGetMenu(), "悬浮窗皮肤", Utf82QString(msg));

        if (qSkinName.isEmpty()) {
          glb->glbShowMsg("取消成功");
          continue;
        }

        for (auto qobj: m_ChooseSkinMenu->actions()) {
          if (qobject_cast<QAction*>(qobj)->text() == qSkinName) {
            glb->glbShowMsg("请勿输入已经存在的名称！");
            continue;
          }
        }

        file.make_preferred();
        AddSkin(qSkinName, file);
      }
    }
  };
  m_Followers.insert(&setDropSkin);

  auto& followers = glb->glbGetSystemTray()->m_Followers;
  followers.insert(&m_ActiveWinodow);
}

QAction* NeoSpeedboxPlg::InitMenuAction()
{
  auto action = m_MainMenu->addAction("网卡选择");
  auto menu = new QMenu(m_MainMenu);
  menu->setAttribute(Qt::WA_TranslucentBackground, true);
  menu->setToolTipsVisible(true);
  action->setMenu(menu);
  m_Speedbox = new SpeedBox(this, m_Settings, menu);
  AddMainObject(m_Speedbox);   // 添加到对象列表
  this->PluginObject::InitMenuAction();
  LoadHideAsideMenu(m_MainMenu);

  m_MainMenu->addSeparator();
  LoadChooseSkinMenu(m_MainMenu);
  m_MainMenu->addAction("皮肤选择")->setMenu(m_ChooseSkinMenu);
  AddSkinConnect(m_MainMenu->addAction("添加皮肤"));
  LoadRemoveSkinMenu(m_MainMenu);
  m_MainMenu->addAction("皮肤删除")->setMenu(m_RemoveSkinMenu);

  m_Speedbox->InitShow();
  bool buffer = false;
  const auto& info = m_PluginMethod[u8"enableBlur"];
  info.function(PluginEvent::BoolGet, &buffer);
  info.function(PluginEvent::Bool, &buffer);
  
  return nullptr;
}

YJson& NeoSpeedboxPlg::InitSettings(YJson& settings)
{
  if (!settings.isObject()) {
    settings = YJson::O {
      {u8"ShowForm", true},
      {u8"Position", YJson::A { 100, 100 }},
      {u8"HideAside", 15},     // 0xFF
      {u8"ColorEffect", 0,},
      {u8"BackgroundColorRgba", YJson::A { 51, 51, 119, 204 } },
      {u8"CurSkin", u8"经典火绒"},
      {u8"UserSkins", YJson::O {}},
      {u8"NetCardDisabled", YJson::A {}},
    };
  }
  std::u8string default_skins[][2] = {
    {u8"经典火绒"s, u8":/skins/Huorong.ui"s},
    {u8"电脑管家"s, u8":/skins/Guanjia.ui"s},
    {u8"数字卫士"s, u8":/skins/360.ui"s},
    {u8"独霸一方"s, u8":/skins/duba.ui"s},
    {u8"果里果气"s, u8":/skins/Fruit.ui"s},
    {u8"开源力量"s, u8":/skins/archlinux.ui"s},
  };
  auto& skins = settings[u8"UserSkins"];
  if (!skins.isObject()) skins = YJson::Object;
  for (size_t i=0; i!=6; ++i) {
    skins[default_skins[i][0]] = default_skins[i][1];
  }
  return settings;
  // we may not need to call SaveSettings;
}

void NeoSpeedboxPlg::LoadRemoveSkinMenu(QMenu* parent)
{
  m_RemoveSkinMenu = new QMenu(parent);
  m_RemoveSkinMenu->setAttribute(Qt::WA_TranslucentBackground, true);
  m_RemoveSkinMenu->setToolTipsVisible(true);
  const auto& curSkin = m_Settings[u8"CurSkin"].getValueString();
  for (const auto& [name, path]: m_Settings[u8"UserSkins"].getObject()) {
    auto const action = m_RemoveSkinMenu->addAction(PluginObject::Utf82QString(name));
    action->setToolTip(PluginObject::Utf82QString(path.getValueString()));
    RemoveSkinConnect(action);
  }
}

void NeoSpeedboxPlg::LoadChooseSkinMenu(QMenu* parent)
{
  m_ChooseSkinMenu = new QMenu(parent);
  m_ChooseSkinGroup = new QActionGroup(m_ChooseSkinMenu);
  m_ChooseSkinMenu->setToolTipsVisible(true);
  m_ChooseSkinMenu->setAttribute(Qt::WA_TranslucentBackground, true);

  const std::u8string& curSkin = m_Settings[u8"CurSkin"].getValueString();
  for (const auto& [name, path]: m_Settings[u8"UserSkins"].getObject()) {
    auto const action = m_ChooseSkinMenu->addAction(PluginObject::Utf82QString(name));
    const auto qname = action->text();
    action->setCheckable(true);
    m_ChooseSkinGroup->addAction(action);
    action->setChecked(curSkin == name);
    action->setToolTip(PluginObject::Utf82QString(path.getValueString()));
    ChooseSkinConnect(action);
  }
}

void NeoSpeedboxPlg::LoadHideAsideMenu(QMenu* parent)
{

  const auto menu = new QMenu(parent);
  menu->setToolTipsVisible(true);
  menu->setAttribute(Qt::WA_TranslucentBackground, true);
  parent->addAction("贴边隐藏")->setMenu(menu);

  auto lst = {"上", "右", "下", "左"};
  const int32_t set = m_Settings[u8"HideAside"].getValueInt();
  int32_t bit = 1;
  for (auto i: lst) {
    auto const action = menu->addAction(i);
    action->setCheckable(true);
    action->setChecked(set & bit);
    QObject::connect(action, &QAction::triggered, menu, [this, bit](bool on){
      auto& obj = m_Settings[u8"HideAside"];
      obj = on ? (obj.getValueInt() | bit) : (obj.getValueInt() & ~bit);
      mgr->SaveSettings();
      glb->glbShowMsg("设置成功！");
    });
    bit <<= 1;
  }
}

void NeoSpeedboxPlg::RemoveSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, m_RemoveSkinMenu, [action, this](){
    const auto qname = action->text();
    const auto name = PluginObject::QString2Utf8(qname);
    if (name == m_Settings[u8"CurSkin"].getValueString()) {
      glb->glbShowMsg("当前皮肤正在使用，无法删除！");
      return;
    }
    m_Settings[u8"UserSkins"].remove(name);
    mgr->SaveSettings();
    m_RemoveSkinMenu->removeAction(action);

    QAction* anotherAction = nullptr;
    for (auto i: m_ChooseSkinGroup->actions()) {
      if (i->text() != qname) continue;
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
      // const auto qname = action->text();
      const auto qSkinName = QInputDialog::getText(glb->glbGetMenu(), "悬浮窗皮肤", "请输入皮肤昵称：");
      if (qSkinName.isEmpty()) {
        glb->glbShowMsg("取消成功");
        return;
      }

      for (auto qobj: m_ChooseSkinMenu->actions()) {
        if (qobject_cast<QAction*>(qobj)->text() == qSkinName) {
          glb->glbShowMsg("请勿输入已经存在的名称！");
          return;
        }
      }

      const auto qFilePath = QFileDialog::getOpenFileName(glb->glbGetMenu(), "选择文件", ".", "(*.ui)");
      if (qFilePath.isEmpty()) {
        glb->glbShowMsg("取消成功");
        return;
      }
      fs::path path = qFilePath.toStdU16String();
      path.make_preferred();
      if (!path.has_filename()) {
        glb->glbShowMsg("添加失败！");
        return;
      }

      AddSkin(qSkinName, path);
  });
}

void NeoSpeedboxPlg::AddSkin(const QString& name, const fs::path& path)
{

  if (!fs::exists("skins")) {
    fs::create_directory("skins");
  }

  auto u8FilePath = u8"skins" / path.filename();
  u8FilePath.make_preferred();
  if (fs::absolute(u8FilePath) != path) {
    fs::copy(path, u8FilePath);
  }

  m_Settings[u8"UserSkins"].append(u8FilePath.u8string(), QString2Utf8(name));
  mgr->SaveSettings();

  auto action = m_ChooseSkinMenu->addAction(name);
  action->setToolTip(QString::fromStdU16String(path.u16string()));
  action->setCheckable(true);
  ChooseSkinConnect(action);

  action = m_RemoveSkinMenu->addAction(name);
  RemoveSkinConnect(action);
  glb->glbShowMsg("添加皮肤" + name + "成功！");
}

void NeoSpeedboxPlg::LoadFonts() {
  QFontDatabase::addApplicationFont(":/fonts/Nickainley-Normal-small.ttf");
  QFontDatabase::addApplicationFont(":/fonts/Carattere-Regular-small.ttf");
}
