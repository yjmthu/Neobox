#include <pluginobject.h>
#include <pluginmgr.h>

#include <QMenu>
#include <QAction>

#include <vector>

extern PluginMgr* mgr;

PluginObject::PluginObject(YJson& settings, std::u8string pluginName, std::u8string friendlyName):
  m_Settings(settings),
  m_PlugInfo(PlugInfo { friendlyName, pluginName, m_FunctionMapVoid, m_FunctionMapBool })
{
}

void PluginObject::InitMenuAction(QMenu* pluginMenu)
{
  for (const auto& [_, funInfo]: m_FunctionMapVoid) {
    QObject::connect(pluginMenu->addAction(
          Utf82QString(funInfo.friendlyName)),
        &QAction::triggered, pluginMenu, funInfo.function);
  }
  for (const auto& [_, funInfo]: m_FunctionMapBool) {
    auto const action = pluginMenu->addAction(
          Utf82QString(funInfo.friendlyName));
    action->setCheckable(true);
    action->setChecked(funInfo.status());
    QObject::connect(action,
        &QAction::triggered, pluginMenu, funInfo.function);
  }
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
  mgr->m_MainObjects[m_PlugInfo.m_PluginName] = object;
}

void PluginObject::RemoveMainObject()
{
  mgr->m_MainObjects[m_PlugInfo.m_PluginName] = nullptr;
}

QObject* PluginObject::GetMainObject(const std::u8string& pluginName)
{
  auto const iter = mgr->m_MainObjects.find(pluginName);
  return iter ==mgr->m_MainObjects.end() ? nullptr : iter->second;
}
