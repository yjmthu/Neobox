#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <glbobject.h>
#include <systemapi.h>

#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QActionGroup>

#include <windows.h>
#include <filesystem>

#include "../widgets/plugincenter.hpp"

namespace fs = std::filesystem;
GlbObject *glb;
PluginMgr *mgr;

// #define WRITE_LOG(x) writelog((x))
#define WRITE_LOG(x) 

void writelog(std::string data)
{
  std::ofstream file("neobox.log", std::ios::app);
  file.write(data.data(), data.size());
  file.close();
}


PluginMgr::PluginMgr(GlbObject* glb, QMenu* pluginMainMenu):
  m_GlbObject(glb),
  m_SettingFileName(u8"PluginSettings.json"),
  SaveSettings([this](){m_Settings->toFile(m_SettingFileName, false, YJson::UTF8);}),
  m_MainMenu(pluginMainMenu)
{
  mgr = this;
  if (!fs::exists("plugins")) {
    fs::create_directory("plugins");
  }
  if (fs::exists(m_SettingFileName)) {
    m_Settings = new YJson(m_SettingFileName, YJson::UTF8);
  } else {
    m_Settings = new YJson{ YJson::O{
      { u8"Plugins", YJson::O {
      }},
      { u8"EventMap", YJson::A {
        // YJson::O {
        //   {u8"from", u8"" },
        //   {u8"event", u8"" },
        //   {u8"to", u8"" },
        //   {u8"function", u8""},
        // }
      }},
      { u8"PluginsConfig", YJson::O {
      }},
    }};
  }
}

PluginMgr::~PluginMgr()
{
  for (auto& [_, info]: m_Plugins) {
    if (!info.plugin) continue;
    delete info.plugin;
    info.plugin = nullptr;
    FreeLibrary(reinterpret_cast<HINSTANCE>(info.handle));
  }
  delete m_Settings;
}

void PluginMgr::LoadManageAction(QAction *action)
{
  QObject::connect(action, &QAction::triggered, m_MainMenu, [this](){
    auto instance = PluginCenter::m_Instance;
    if (instance) {
      instance->activateWindow();
      return;
    }
    PluginCenter center(m_Settings->find(u8"Plugins")->second);
    center.exec();
  });
}

const YJson& PluginMgr::GetPluginsInfo() const
{
  return m_Settings->find(u8"Plugins")->second;
}

void PluginMgr::LoadPlugins()
{
  for (auto& [i, j]: m_Settings->find(u8"Plugins")->second.getObject()) {
    const auto name = i;
    if (!j[u8"Enabled"].isTrue()) continue;
    auto& pluginSttings = m_Settings->find(u8"PluginsConfig")->second[name];
    if (!LoadPlugin(name, m_Plugins[name])) {
      m_Plugins.erase(name);
      j[u8"Enabled"] = false;
    }
  }
  SaveSettings();
  InitBroadcast();
}

bool PluginMgr::TooglePlugin(const std::u8string& pluginName, bool on)
{
  auto& info = m_Plugins[pluginName];
  auto& enabled = m_Settings->find(u8"Plugins")->second[pluginName][u8"Enabled"];
  enabled = on;
  SaveSettings();      // 尽量早保存，以免闪退

  if (on) {
    if (LoadPlugin(pluginName, info)) {
      UpdateBroadcast(info.plugin);
    } else {
      m_GlbObject->glbShowMsg("设置失败！");
      enabled = false;
      return false;
    }
  } else {
    FreePlugin(info);
    m_Plugins.erase(pluginName);
  }
  m_GlbObject->glbShowMsg("设置成功！");
  return true;
}

bool PluginMgr::LoadPlugin(std::u8string pluginName, PluginMgr::PluginInfo& pluginInfo)
{
  pluginInfo.plugin = nullptr;
  pluginInfo.handle = nullptr;

  PluginObject* (*newPlugin)(YJson&, PluginMgr*)= nullptr;

// #ifdef _DEBUG
//   fs::path path = __FILEW__;
//   path = path.parent_path().parent_path().parent_path() / "build/plugins";
// #else
  fs::path path = u8"plugins";
  path /= pluginName;
  if (!LoadPlugEnv(path)) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件文件夹加载失败！"));
    return false;
  }
// #endif
  path /= pluginName + u8".dll";
  if (!fs::exists(path)) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件文件加载失败！"));
    return false;
  }
  path.make_preferred();
  std::wstring wPath = path.wstring();
  wPath.push_back(L'\0');
  HINSTANCE hdll = LoadLibraryW(wPath.data());
  if (!hdll) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件动态库加载失败！"));
    return false;
  }
  pluginInfo.handle = hdll;
  newPlugin = reinterpret_cast<decltype(newPlugin)>(GetProcAddress(hdll, "newPlugin"));
  if (!newPlugin) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件函数加载失败！"));
    FreeLibrary(hdll);
    return false;
  }
  try {
    pluginInfo.plugin = newPlugin(m_Settings->find(u8"PluginsConfig")->second[pluginName], this);      // nice
    auto const mainMenuAction = pluginInfo.plugin->InitMenuAction();
    if (mainMenuAction) {
      mainMenuAction->setProperty("pluginName", QString::fromUtf8(pluginName.data(), pluginName.size()));
      glb->glbGetMenu()->addAction(mainMenuAction);
    }
    return true;
  } catch (...) {
    pluginInfo.plugin = nullptr;
    pluginInfo.handle = nullptr;
    FreeLibrary(hdll);
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件初始化失败！"));
  }
  return false;
}

bool PluginMgr::FreePlugin(PluginInfo& info)
{
  delete info.plugin;
  info.plugin = nullptr;
  return FreeLibrary(reinterpret_cast<HINSTANCE>(info.handle));
}

void PluginMgr::InitBroadcast()
{
  for (auto& [name, info]: m_Plugins) {
    if (!info.plugin) continue;
    for (const auto& idol: info.plugin->m_Following) {
      auto iter = m_Plugins.find(idol.first);
      if (iter == m_Plugins.end() || !iter->second.plugin) continue;
      iter->second.plugin->m_Followers.insert(&idol.second);
    }
  }
}

void PluginMgr::UpdateBroadcast(PluginObject* plugin)
{
  for (auto& [name, info]: m_Plugins) {
    if (!info.plugin || info.plugin == plugin) continue;
    auto const & lst = info.plugin->m_Following;
    auto iter = std::find_if(lst.begin(), lst.end(),
      [plugin](decltype(lst.front())& item){
        return item.first == plugin->m_PluginName;
      });
    if (iter == lst.end()) continue;
    plugin->m_Followers.insert(&iter->second);
  }
  for (auto& idol: plugin->m_Following) {
    auto iter = m_Plugins.find(idol.first);
    if (iter == m_Plugins.end() || !iter->second.plugin) continue;
    iter->second.plugin->m_Followers.insert(&idol.second);
  }
}

bool PluginMgr::LoadPlugEnv(const fs::path& dir)
{
  if (!fs::exists(dir)) {
    WRITE_LOG("dir not exsist\n");
    return false;
  }
  constexpr auto const varName = L"PATH";
  std::wstring strEnvPaths(GetEnvironmentVariableW(varName, nullptr, 0), L'\0');  
  auto const dwSize = GetEnvironmentVariableW(varName, strEnvPaths.data(), strEnvPaths.size());
  strEnvPaths.pop_back();
  if (!strEnvPaths.empty() && !strEnvPaths.ends_with(L';'))
    strEnvPaths.push_back(L';');
  auto path = fs::absolute(dir);
  path.make_preferred();
  auto wpath = path.wstring();
  for (auto pos = strEnvPaths.find(wpath); pos != std::wstring::npos; pos = strEnvPaths.find(wpath, pos)) {
    pos += wpath.size();
    if (pos == strEnvPaths.size() || strEnvPaths[pos] == L';') {
      return true;
    }
  }
  wpath.push_back(L'\0');
  strEnvPaths.append(wpath);
  auto const bRet = SetEnvironmentVariableW(varName, strEnvPaths.data());  
  return bRet;
}


bool PluginMgr::InstallPlugin(const std::u8string& plugin, const YJson& info)
{
  auto iter = m_Plugins.find(plugin);
  if (iter != m_Plugins.end() && iter->second.plugin) {
    return false;
  }
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  auto infoIter = pluginsInfo.find(plugin);

  if (infoIter != pluginsInfo.endO()) {
    return false;
  }

  auto& pluginInfo = pluginsInfo[plugin] = info;
  pluginInfo[u8"Enabled"] = false;
  SaveSettings();
  return true;
}

bool PluginMgr::UnInstallPlugin(const std::u8string& plugin)
{
  auto iter = m_Plugins.find(plugin);
  if (iter != m_Plugins.end() && iter->second.plugin) {
    FreePlugin(iter->second);
    m_Plugins.erase(iter);
  }
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  auto infoIter = pluginsInfo.find(plugin);

  if (infoIter == pluginsInfo.endO()) {
    return false;
  }
  pluginsInfo.remove(infoIter);
  SaveSettings();
  return true;
}


bool PluginMgr::UpdatePlugin(const std::u8string& plugin, const YJson* info)
{
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  auto infoIter = pluginsInfo.find(plugin);

  if (infoIter == pluginsInfo.endO()) {
    return false;
  }
  auto& pluginInfo = infoIter->second;

  if (info) {
    if (const auto iter = m_Plugins.find(plugin); iter != m_Plugins.end() && iter->second.plugin) {
      return false;
    }
    auto& enabled = (pluginInfo = *info)[u8"Enabled"];
    if (enabled.isTrue()) {
      auto& ptrInfo = m_Plugins[plugin];
      if (LoadPlugin(plugin, ptrInfo)) {
        UpdateBroadcast(ptrInfo.plugin);
      } else {
        enabled = false;
        SaveSettings();
        return false;
      }
    }
    SaveSettings();
  } else {
    if (const auto iter = m_Plugins.find(plugin); iter != m_Plugins.end() && iter->second.plugin) {
      FreePlugin(iter->second);
      m_Plugins.erase(iter);
    }
    pluginInfo[u8"Enabled"] = false;
  }
  return true;
}

void PluginMgr::UpdatePluginOrder(YJson&& data)
{
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  pluginsInfo = std::move(data);
  std::map<std::u8string, QAction*> actions;

  auto const mainMenu = glb->glbGetMenu();
  for (auto action: mainMenu->actions()) {
    auto pluginName = action->property("pluginName");
    if (!pluginName.isNull()) {
      actions[PluginObject::QString2Utf8(pluginName.toString())] = action;
    }
  }

  for (auto& [name, action]: actions) {
    mainMenu->removeAction(action);
  }

  for (auto& [name, info]: pluginsInfo.getObject()) {
    mainMenu->addAction(actions[name]);
  }

  glb->glbShowMsg("调整成功！");
  SaveSettings();
}

bool PluginMgr::IsPluginEnabled(const std::u8string& plugin) const
{
  return m_Plugins.find(plugin) != m_Plugins.end();
}
