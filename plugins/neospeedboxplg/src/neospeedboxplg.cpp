#include <neospeedboxplg.h>
#include <speedbox.h>
#include <pluginobject.h>
#include <yjson.h>
#include <systemapi.h>
#include <neosystemtray.hpp>
#include <netspeedhelper.h>
#include <neomenu.hpp>

#include <menubase.hpp>
#include <QActionGroup>
#include <QInputDialog>
#include <QFileDialog>
// #include <QFontDatabase>
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QWindow>

#ifdef _WIN32
#include <Windows.h>
#else
// #include <KF5/KWindowSystem/KWindowEffects>
#endif

#include <filesystem>
#include <ranges>

#define PluginName NeoSpeedboxPlg
#include <pluginexport.cpp>

namespace fs = std::filesystem;
using namespace std::literals;

PluginName::PluginName(YJson& settings)
  : PluginObject(InitSettings(settings), u8"neospeedboxplg", u8"网速悬浮")
  , m_Settings(settings)
  , m_NetSpeedHelper(new NetSpeedHelper(m_Settings.GetNetCardDisabled()))
{
  // LoadFonts();
  InitFunctionMap();
}

PluginName::~PluginName()
{
  RemoveMainObject();
  auto& followers = mgr->m_Tray->m_Followers;
  followers.erase(&m_ActiveWinodow);
  delete m_Speedbox;
  delete m_NetSpeedHelper;
}

void PluginName::InitFunctionMap() {
  m_PluginMethod = {
    {u8"moveLeftTop", {
      u8"还原位置", u8"将窗口移动到左上方位置", [this](PluginEvent, void*){
        m_Speedbox->InitMove();
      }, PluginEvent::Void}
    },
    {u8"enableBlur", {
      u8"模糊背景", u8"Windows10+或KDE下的模糊效果", [this](PluginEvent event, void* data){
        if (event == PluginEvent::Bool) {
          m_Settings.SetColorEffect(*reinterpret_cast<bool *>(data));
#ifdef _WIN32
          auto status = *reinterpret_cast<bool *>(data) ? ACCENT_ENABLE_BLURBEHIND : ACCENT_DISABLED;
          auto hWnd = reinterpret_cast<HWND>(m_Speedbox->winId());
          const QColor col(Qt::transparent);
          SetWindowCompositionAttribute(hWnd, status,
            qRgba(col.blue(), col.green(), col.red(), col.alpha()));
#else
          // KWindowEffects::enableBlurBehind(
          //   qobject_cast<QWindow*>(m_Speedbox), *reinterpret_cast<bool *>(data));
          mgr->ShowMsg("暂不可用");
#endif
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool *>(data) = m_Settings.GetColorEffect();
        }
      }, PluginEvent::Bool}
    },
    {u8"enableMousePenetrate", {
      u8"鼠标穿透", u8"鼠标操作穿透悬浮窗", [this](PluginEvent event, void* data){
        if (event == PluginEvent::Bool) {
          auto const on = *reinterpret_cast<bool *>(data);
          m_Settings.SetMousePenetrate(on);
          delete m_Speedbox;
          m_Speedbox = new SpeedBox(this, m_Settings, m_NetCardMenu);
          AddMainObject(m_Speedbox);       // 添加到对象列表
          m_Speedbox->InitShow(m_PluginMethod[u8"enableBlur"].function);
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool *>(data) = m_Settings.GetMousePenetrate();
        }
      }, PluginEvent::Bool}
    },
    {u8"enableProgressMonitor", {
      u8"进程信息", u8"滚轮查看进程信息", [this](PluginEvent event, void* data){
        if (event == PluginEvent::Bool) {
          auto& on = *reinterpret_cast<bool *>(data);
          m_Settings.SetProgressMonitor(on);
          m_Speedbox->SetProgressMonitor(on);
          mgr->ShowMsg("设置成功！");
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool *>(data) = m_Settings.GetProgressMonitor();
        }
      }, PluginEvent::Bool}
    },
  };

  m_ActiveWinodow = [this](PluginEvent event, void*){
    if (event == PluginEvent::MouseDoubleClick) {
      if (m_Speedbox->isVisible()) {
        m_Speedbox->hide();
        mgr->ShowMsg("隐藏悬浮窗成功！");
      } else {
        m_Speedbox->show();
        mgr->ShowMsg("显示悬浮窗成功！");
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
          return i.isValid() && i.isLocalFile() && i.fileName().endsWith(".dll");
        }) | std::views::transform([](const QUrl& url){
          return fs::path(PluginObject::QString2Utf8(url.toLocalFile()));
        }) | std::views::filter([](const fs::path& url) {
          return fs::exists(url) && url.has_filename();
        });
      
      std::vector<fs::path> vec(uiUrlsView.begin(), uiUrlsView.end());

      for (auto& file: vec) {
        auto const msg = u8"请输入皮肤“" + file.filename().u8string() + u8"”的昵称";
        const auto qSkinName = QInputDialog::getText(mgr->m_Menu, "悬浮窗皮肤", Utf82QString(msg));

        if (qSkinName.isEmpty()) {
          mgr->ShowMsg("取消成功");
          continue;
        }

        for (auto qobj: m_ChooseSkinMenu->actions()) {
          if (qobject_cast<QAction*>(qobj)->text() == qSkinName) {
            mgr->ShowMsg("请勿输入已经存在的名称！");
            continue;
          }
        }

        file.make_preferred();
        AddSkin(qSkinName, file);
      }
    }
  };
  m_Followers.insert(&setDropSkin);

  auto& followers = mgr->m_Tray->m_Followers;
  followers.insert(&m_ActiveWinodow);
}

QAction* PluginName::InitMenuAction()
{
  auto action = m_MainMenu->addAction("网卡选择");
  m_NetCardMenu = new MenuBase(m_MainMenu);
  action->setMenu(m_NetCardMenu);
  m_Speedbox = new SpeedBox(this, m_Settings, m_NetCardMenu);
  AddMainObject(m_Speedbox);   // 添加到对象列表
  this->PluginObject::InitMenuAction();
  LoadHideAsideMenu(m_MainMenu);

  m_MainMenu->addSeparator();
  LoadChooseSkinMenu(m_MainMenu);
  m_MainMenu->addAction("皮肤选择")->setMenu(m_ChooseSkinMenu);
  AddSkinConnect(m_MainMenu->addAction("添加皮肤"));
  LoadRemoveSkinMenu(m_MainMenu);
  m_MainMenu->addAction("皮肤删除")->setMenu(m_RemoveSkinMenu);

  m_Speedbox->InitShow(m_PluginMethod[u8"enableBlur"].function);
  
  return nullptr;
}

YJson& PluginName::InitSettings(YJson& settings)
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
      {u8"MousePenetrate", false},
      {u8"ProgressMonitor", false}
    };
  }
  std::u8string default_skins[][2] = {
    {u8"经典火绒"s, u8"skin1"s},
    {u8"电脑管家"s, u8"skin2"s},
    {u8"数字卫士"s, u8"skin3"s},
    {u8"独霸一方"s, u8"skin4"s},
    {u8"开源力量"s, u8"skin5"s},
    {u8"果里果气"s, u8"skin6"s},
  };
  auto& skins = settings[u8"UserSkins"];
  if (!skins.isObject()) skins = YJson::Object;

  auto& curSkin = settings[u8"CurSkin"].getValueString();
  if (auto iter = skins.find(curSkin); iter == skins.endO()) {
    curSkin = u8"经典火绒";
  } else {
#ifdef _WIN32
    fs::path curSkinPath = u8"skins/" + iter->second.getValueString() + u8".dll";
#else
    fs::path curSkinPath = u8"skins/lib" + iter->second.getValueString() + u8".so";
#endif
    if (!fs::exists(curSkinPath)) {
      skins.remove(curSkin);
      curSkin = u8"经典火绒";
    }
  }
  
  for (size_t i=0; i!=6; ++i) {
    skins[default_skins[i][0]] = default_skins[i][1];
  }

  const auto& curFileName = Utf82QString(skins[curSkin].getValueString());
#ifdef _WIN32
  auto const curSkinPath = "skins/" + curFileName + ".dll";
  auto const curResPath = ":/dlls/" + curFileName + ".dll";
#else
  auto const curSkinPath = "skins/lib" + curFileName + ".so";
  auto const curResPath = ":/dlls/lib" + curFileName + ".so";
#endif
  auto& version = settings[u8"Version"];
  if (!version.isNumber()) {
    version = 0;
    settings[u8"MousePenetrate"] = false;
  }

  if (version.getValueInt() < 1) {
    version = 1;
    settings[u8"ProgressMonitor"] = false;
  }

  return settings;
  // we may not need to call SaveSettings;
}

void PluginName::LoadRemoveSkinMenu(MenuBase* parent)
{
  m_RemoveSkinMenu = new MenuBase(parent);
  const auto curSkin = m_Settings.GetCurSkin();
  for (const auto& [name, path]: m_Settings.GetUserSkins()) {
    auto const action = m_RemoveSkinMenu->addAction(PluginObject::Utf82QString(name));
    action->setToolTip(PluginObject::Utf82QString(path.getValueString()));
    RemoveSkinConnect(action);
  }
}

void PluginName::LoadChooseSkinMenu(MenuBase* parent)
{
  m_ChooseSkinMenu = new MenuBase(parent);
  m_ChooseSkinGroup = new QActionGroup(m_ChooseSkinMenu);

  const std::u8string curSkin = m_Settings.GetCurSkin();
  for (const auto& [name, path]: m_Settings.GetUserSkins()) {
    auto const action = m_ChooseSkinMenu->addAction(PluginObject::Utf82QString(name));
    const auto qname = action->text();
    action->setCheckable(true);
    m_ChooseSkinGroup->addAction(action);
    action->setChecked(curSkin == name);
    action->setToolTip(PluginObject::Utf82QString(path.getValueString()));
    ChooseSkinConnect(action);
  }
}

void PluginName::LoadHideAsideMenu(MenuBase* parent)
{

  const auto menu = new MenuBase(parent);
  parent->addAction("贴边隐藏")->setMenu(menu);

  auto lst = {"上", "右", "下", "左"};
  const int32_t set = m_Settings.GetHideAside();
  int32_t bit = 1;
  for (auto i: lst) {
    auto const action = menu->addAction(i);
    action->setCheckable(true);
    action->setChecked(set & bit);
    QObject::connect(action, &QAction::triggered, menu, [this, bit](bool on){
      auto obj = m_Settings.GetHideAside();
      obj = on ? (obj | bit) : (obj & ~bit);
      m_Settings.SetHideAside(obj);
      mgr->ShowMsg("设置成功！");
    });
    bit <<= 1;
  }
}

void PluginName::RemoveSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, m_RemoveSkinMenu, [action, this](){
    const auto qname = action->text();
    const auto name = PluginObject::QString2Utf8(qname);
    if (name == m_Settings.GetCurSkin()) {
      mgr->ShowMsg("当前皮肤正在使用，无法删除！");
      return;
    }
    YJson skins(m_Settings.GetUserSkins());
    skins.remove(name);
    m_Settings.SetUserSkins(std::move(skins.getObject()));
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
    mgr->ShowMsg("删除皮肤成功！");
  });
}

void PluginName::ChooseSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, m_ChooseSkinMenu, [action, this](){
    m_Settings.SetCurSkin(PluginObject::QString2Utf8(action->text()));
    if (m_Speedbox->UpdateSkin()) {
      mgr->ShowMsg("更换皮肤成功！");
    } else {
      mgr->ShowMsg("更换皮肤失败！");
    }
  });
}

void PluginName::AddSkinConnect(QAction* action)
{
  QObject::connect(action, &QAction::triggered, [action, this](){
      // const auto qname = action->text();
      const auto qSkinName = QInputDialog::getText(mgr->m_Menu, "悬浮窗皮肤", "请输入皮肤昵称：");
      if (qSkinName.isEmpty()) {
        mgr->ShowMsg("取消成功");
        return;
      }

      for (auto qobj: m_ChooseSkinMenu->actions()) {
        if (qobject_cast<QAction*>(qobj)->text() == qSkinName) {
          mgr->ShowMsg("请勿输入已经存在的名称！");
          return;
        }
      }

      const auto qFilePath =
        QFileDialog::getOpenFileName(mgr->m_Menu, "选择文件", ".", "(*.dll)");
      if (qFilePath.isEmpty()) {
        mgr->ShowMsg("取消成功");
        return;
      }
      fs::path path = qFilePath.toStdU16String();
      path.make_preferred();
      if (!path.has_filename()) {
        mgr->ShowMsg("添加失败！");
        return;
      }

      AddSkin(qSkinName, path);
  });
}

void PluginName::AddSkin(const QString& name, const fs::path& path)
{

  if (!fs::exists("skins")) {
    fs::create_directory("skins");
  }

  auto u8FilePath = u8"skins" / path.filename();
  u8FilePath.make_preferred();
  if (fs::absolute(u8FilePath) != path) {
    fs::copy(path, u8FilePath);
  }

  const auto fileName = path.stem();
  m_Settings.AppendUserSkins(QString2Utf8(name), fileName.u8string());

  auto action = m_ChooseSkinMenu->addAction(name);
  action->setToolTip(QString::fromStdU16String(fileName.u16string()));
  action->setCheckable(true);
  ChooseSkinConnect(action);

  action = m_RemoveSkinMenu->addAction(name);
  RemoveSkinConnect(action);
  mgr->ShowMsg("添加皮肤" + name + "成功！");
}

// void NeoSpeedboxPlg::LoadFonts() {
//   QFontDatabase::addApplicationFont(":/fonts/Nickainley-Normal-small.ttf");
//   QFontDatabase::addApplicationFont(":/fonts/Carattere-Regular-small.ttf");
// }
