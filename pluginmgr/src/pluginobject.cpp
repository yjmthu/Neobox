#include <pluginobject.h>
#include <pluginmgr.h>

#include <QMenu>
#include <QAction>

#include <vector>

extern PluginMgr* mgr;

PluginObject::PluginObject(YJson& settings, std::u8string name,
  const std::u8string& friendlyName):
  m_Settings(settings),
  m_PluginName(std::move(name)),
  m_MainAction(mgr->m_MainMenu->addAction((Utf82QString(friendlyName)))),
  m_MainMenu(new QMenu(mgr->m_MainMenu)),
  m_PluginMethod(PluginMethod { m_FunctionMapVoid, m_FunctionMapBool })
{
  m_MainMenu->setAttribute(Qt::WA_TranslucentBackground, true);
  m_MainMenu->setToolTipsVisible(true);
  m_MainAction->setMenu(m_MainMenu);
}

PluginObject::~PluginObject()
{
  delete m_MainMenu;
  delete m_MainAction;
}

void PluginObject::InitMenuAction()
{
  for (const auto& [_, funInfo]: m_FunctionMapVoid) {
    auto const action = m_MainMenu->addAction(
          Utf82QString(funInfo.friendlyName));
    action->setToolTip(PluginObject::Utf82QString(funInfo.description));
    QObject::connect(action,
        &QAction::triggered, m_MainMenu, funInfo.function);
  }
  for (const auto& [_, funInfo]: m_FunctionMapBool) {
    auto const action = m_MainMenu->addAction(
          Utf82QString(funInfo.friendlyName));
    action->setCheckable(true);
    action->setChecked(funInfo.status());
    action->setToolTip(PluginObject::Utf82QString(funInfo.description));
    QObject::connect(action,
        &QAction::triggered, m_MainMenu, funInfo.function);
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
  mgr->m_MainObjects[m_PluginName] = object;
}

void PluginObject::RemoveMainObject()
{
  mgr->m_MainObjects[m_PluginName] = nullptr;
}

QObject* PluginObject::GetMainObject(const std::u8string& pluginName)
{
  auto const iter = mgr->m_MainObjects.find(pluginName);
  return iter ==mgr->m_MainObjects.end() ? nullptr : iter->second;
}
