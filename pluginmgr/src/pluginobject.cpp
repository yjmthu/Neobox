#include <neobox/pluginobject.h>
#include <neobox/pluginmgr.h>
#include <neobox/neomenu.hpp>

#include <neobox/menubase.hpp>
#include <QAction>

extern PluginMgr* mgr;

PluginObject::PluginObject(YJson& settings, std::u8string name,
  const std::u8string& friendlyName):
  m_Settings(settings),
  m_PluginName(std::move(name)),
  m_MainAction(mgr->m_Menu->m_PluginMenu->addAction((Utf82QString(friendlyName)))),
  m_MainMenu(new MenuBase(mgr->m_Menu->m_PluginMenu)) //,
{
  m_MainAction->setMenu(m_MainMenu);
}

PluginObject::~PluginObject()
{
  for (const auto& idol: m_Following) {
    auto& info = mgr->m_Plugins[idol.first];
    if (!info.plugin) continue;
    auto& lst = info.plugin->m_Followers;
    auto iter = std::find(lst.begin(), lst.end(), &idol.second);
    lst.erase(iter);
  }
  delete m_MainMenu;
  delete m_MainAction;
}

QAction* PluginObject::InitMenuAction()
{
  for (const auto& [_, funInfo]: m_PluginMethod) {
    auto const action = m_MainMenu->addAction(
          Utf82QString(funInfo.friendlyName));
    action->setToolTip(PluginObject::Utf82QString(funInfo.description));
    if (funInfo.type == PluginEvent::Void) {
      QObject::connect(action, &QAction::triggered, m_MainMenu, std::bind(funInfo.function, PluginEvent::Void, nullptr));
    } else if (funInfo.type == PluginEvent::Bool) {
      bool status = false;
      action->setCheckable(true);
      const auto& func = funInfo.function;
      func(PluginEvent::BoolGet, &status);
      action->setChecked(status);
      QObject::connect(action, &QAction::triggered, m_MainMenu, [func, action](bool on){
        auto const last = on;
        func(PluginEvent::Bool, &on);
        if (last != on) action->setChecked(on);
      });
    }
  }
  return nullptr;
}

std::u8string PluginObject::QString2Utf8(const QString& str)
{
  const QByteArray& array = str.toUtf8();
  return std::u8string(array.begin(), array.end());
}

QString PluginObject::Utf82QString(const std::u8string& str)
{
  return QString::fromUtf8(str.data(), str.size());
}

void PluginObject::AddMainObject(QObject* object)
{
  mgr->m_MainObjects[m_PluginName] = object;
}

void PluginObject::RemoveMainObject()
{
  mgr->m_MainObjects.erase(m_PluginName);
  // mgr->m_MainObjects[m_PluginName] = nullptr;
}

void PluginObject::SendBroadcast(PluginEvent event, void* data)
{
  for (auto const fun: m_Followers) {
    fun->operator()(event, data);
  }
}

QObject* PluginObject::GetMainObject(const std::u8string& pluginName)
{
  auto const iter = mgr->m_MainObjects.find(pluginName);
  return iter == mgr->m_MainObjects.end() ? nullptr : iter->second;
}
