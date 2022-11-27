#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>

#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

PluginMgr* mgr;

PluginMgr::PluginMgr(GlbObject* glb, QMenu* pluginMainMenu):
  m_GlbObject(glb),
  m_SettingFileName(u8"PluginSettings.json"),
  SaveSettings([this](){m_Settings->toFile(m_SettingFileName);}),
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
        {u8"neospeedboxplg", true},
        {u8"neotranslateplg", true},
        {u8"neoocrplg", true},
        {u8"neowallpaperplg", true},
        {u8"neosystemplg", true},
      }},
      { u8"KeyMap", YJson::O {
        {u8"neotranslateplg", YJson::A {
          YJson::A {u8"toggleVisibility", 0, YJson::A {u8"Shift", u8"Z"}},
        }},
        {u8"neoocrplg", YJson::A {
          YJson::A {u8"screenfetch", 0, YJson::A {"Ctrl", u8"Shift", u8"A"}},
        }},
      }},
      { u8"PluginsConfig", YJson::O {
      }},
    }};
  }
  LoadPlugins();
  InitBroadcast();
}

PluginMgr::~PluginMgr()
{
  for (auto [i, j]: m_PluginPath) {
    if (j) delete i;
    FreeLibrary(reinterpret_cast<HINSTANCE>(j));
  }
  delete m_Settings;
}

void PluginMgr::LoadPlugins()
{
  for (const auto& [name, on]: m_Settings->find(u8"Plugins")->second.getObject()) {
    if (!on.isTrue()) continue;
    PluginObject* objptr = nullptr;
    auto& pluginSttings = m_Settings->find(u8"PluginsConfig")->second[name];
    objptr = LoadPlugin(name);
    if (!objptr) continue;
    auto& pluginName = objptr->GetPlugInfo().m_FriendlyName;
    QMenu* const pluginMenu = new QMenu(m_MainMenu);
    pluginMenu->setAttribute(Qt::WA_TranslucentBackground, true);
    pluginMenu->setToolTipsVisible(true);
    m_MainMenu->addAction(PluginObject::Utf82QString(pluginName))->setMenu(pluginMenu);
    objptr->InitMenuAction(pluginMenu);
    m_Plugins[name] = objptr;
  }
  SaveSettings();
}

PluginObject* PluginMgr::LoadPlugin(const std::u8string& pluginName)
{
  PluginObject* (*newPlugin)(YJson&, PluginMgr*)= nullptr;

#ifndef _DEBUG
  fs::path path = u8"plugins";
#else
  fs::path path = __FILEW__;
  path = path.parent_path().parent_path().parent_path() / "build/plugins";
#endif
  path /= pluginName + u8".dll";
  if (!fs::exists(path)) return nullptr;
  path.make_preferred();
  std::wstring wPath = path.wstring();
  wPath.push_back(L'\0');
  HINSTANCE hdll = LoadLibraryW(wPath.data());
  if (!hdll) return nullptr;
  newPlugin = reinterpret_cast<decltype(newPlugin)>(GetProcAddress(hdll, "newPlugin"));
  if (!newPlugin) {
    FreeLibrary(hdll);
  }
  auto plugin = newPlugin(m_Settings->find(u8"PluginsConfig")->second[pluginName], this);      // nice
  m_PluginPath[plugin] = hdll;
  return plugin;
}

void PluginMgr::FreePlugin(PluginObject* plugin)
{
  delete plugin;
  auto& hdll = m_PluginPath[plugin];
  FreeLibrary(reinterpret_cast<HINSTANCE>(hdll));
  hdll = nullptr;
}

void PluginMgr::InitBroadcast()
{
  for (auto& [name, plugin]: m_Plugins) {
    for (const auto& [idol, fun]: plugin->m_Following) {
      auto iter = m_Plugins.find(idol);
      if (iter == m_Plugins.end()) continue;
      iter->second->m_Followers.push_back(&fun);
    }
  }
}
